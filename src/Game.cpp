#include "Game.h"
#include "Platform.h"
#include "Math.h"
#include "Renderer.h"
#include "World.h"

#include "Unity.cpp"

#define CONSOLE_LOG_SIZE 64
#define CONSOLE_LINE_SIZE 256

struct ConsoleState {
    bool32 active;
    char input[256];
    u32 inputCount;

    r32 animationProgress;
    r32 animationSpeed;

    char log[CONSOLE_LOG_SIZE][CONSOLE_LINE_SIZE];
    i32 logNext;
};

void WriteConsole(ConsoleState *console, char *text);

typedef struct {
    bool initialized;
    RenderContext *renderer;
    MemorySegment game_memory;
    MemorySegment renderer_memory;
    GameAssets assets;
    World *world;
    bool32 shutdown;

    r32 elapsedTime;
    ConsoleState console;
} GameState;

static void
GameInit(GameState *state, PlatformAPI *api, GameMemory *memory) {
    MemorySegment mem_all = {};
    mem_all.size = memory->permanentSize - sizeof(GameState);
    mem_all.base = (u8*)memory->permanent + sizeof(GameState);
    mem_all.used = 0;
    state->game_memory = mem_all;

    MemorySegment mem_transient = {};
    mem_transient.size = memory->transientSize;
    mem_transient.base = (u8*)memory->transient;
    mem_transient.used = 0;
    state->assets = AssetsInit(api, mem_transient);

    // Allocate memory for renderer
    MemorySegment renderer_memory = AllocMemory(&mem_all, 16 * 1024 * 1024);
    state->renderer_memory = renderer_memory;
    state->renderer = RenderInit(&state->assets, renderer_memory);

    MemorySegment world_memory = AllocMemory(&mem_all, sizeof(World));
    state->world = (World*)world_memory.base;
    WorldCreate(state->world);

    state->console.animationProgress = 0.0f;
    state->console.animationSpeed = 0.05f;

    state->elapsedTime = 0.0f;

    state->shutdown = false;
    state->initialized = true;

    WriteConsole(&state->console, "Game initialized.");

    char *memUsedText = mprintf("Used memory: %d MB", mem_all.used / (1024 * 1024));
    SCOPE_FREE(memUsedText);
    WriteConsole(&state->console, memUsedText);

    char *memText = mprintf("Free memory: %d MB", (mem_all.size - mem_all.used) / (1024 * 1024));
    SCOPE_FREE(memText);
    WriteConsole(&state->console, memText);

    char *memTotalText = mprintf("Total memory: %d MB", mem_all.size / (1024 * 1024));
    SCOPE_FREE(memTotalText);
    WriteConsole(&state->console, memTotalText);
}

void ProcessCommand(GameState *state, const char *input) {
   
    const char *src = input;
    char buff[256];
    for (i32 i = 0; i < 255; ++i) {
        buff[i] = *src++;
        if (*src == '\r' || *src == ' ' || *src == 0) {
            buff[i+1] = 0;
            break;
        }
    }

    if (strcmp("quit", buff) == 0) {
        state->shutdown = true;
    } else if (strcmp("reload", buff) == 0) {
        WriteConsole(&state->console, "Reloading...");
    } else if (strcmp("clear", buff) == 0) {
        for (i32 i = 0; i < CONSOLE_LOG_SIZE; ++i) {
            state->console.log[i][0] = 0;
        }
    } else {
        char *output = mprintf("Unrecognized command '%s'", buff);
        SCOPE_FREE(output);
        WriteConsole(&state->console, output);
    }
}


void DrawConsole(ConsoleState *console, RenderContext *ctx, v2i screenSize) {

    if (console->animationProgress <= 0.0f) {
        return;
    }

    i32 width = screenSize.x;
    i32 height = (i32)(sin(console->animationProgress * 3.1415f / 2.0f) * screenSize.y / 2);

    Rect2Di targetRect = { 0, 0, width, height  };
    Color bgcolor = { .25f, .35f, .35f, 0.5f };
    DrawSolidRect(ctx, targetRect, bgcolor);
    
    // Draw input in buffer
    i32 index = console->logNext - 1;
    for (i32 i = 0; i < CONSOLE_LOG_SIZE; ++i) {
        // TODO: Dont bother drawing outside screen...
        if (index < 0) {
            index = CONSOLE_LOG_SIZE - 1;
        }

        DrawText(ctx, console->log[index], { 0, height - 40 - 20 * i }, { 0.8f, 0.8f, 0.8f});
        index--;
    }


    Color inputbg = { 0.2f, 0.2f, 0.2f };
    DrawSolidRect(ctx, { 1, height - 19, width - 2, 18}, inputbg);
    DrawText(ctx, ">", { 2, height - 18 }, { 1.0f, 1.0f, 1.0f });
    DrawText(ctx, console->input, { 20, height - 18 }, { 1.0f, 1.0f, 1.0f });
}

void WriteConsole(ConsoleState *console, char *text) {

    // Note: text will not be 0-terminated
    char *src = text;
    char *dst = console->log[console->logNext];

    u32 length = strlen(text);
    
    for (u32 i = 0; i < min(length, CONSOLE_LINE_SIZE - 1); ++i) {
        dst[i] = src[i];
    }

    dst[min(length, CONSOLE_LINE_SIZE-1)] = 0;

    console->logNext++;
    if (console->logNext >= CONSOLE_LOG_SIZE) {
        console->logNext = 0;
    }

}

void ReadConsoleInput(GameState *state, ConsoleState *console, KeyboardState *keyboard) {
    
    if (!console->active) {
        // Console is not displayed, do not consume input
        return;
    }

    for (i32 i = 0; i < keyboard->keyCount; ++i) {
        u32 key = keyboard->keyStack[i];
        if (key == '\b' && console->inputCount != 0) {
            console->inputCount--;
            console->input[console->inputCount] = 0;
        } else if (key == '\r') {
            // Make sure the string is 0-terminated
            console->input[console->inputCount] = 0;
            WriteConsole(console, console->input);
            ProcessCommand(state, console->input);
            // TODO: Execute command
            console->inputCount = 0;
            console->input[0] = 0;
        } else if (key == '`') {
            // Ignore show/hide console command
        } else {
            console->input[console->inputCount++] = (char)key;
        }
    }

    console->input[console->inputCount + 1] = 0;
}

void UpdateConsole(GameState *state, GameInput *input) {
    ReadConsoleInput(state, &state->console, &input->keyboard);

    if (state->console.active && state->console.animationProgress < 1.0f) {
        state->console.animationProgress += state->console.animationSpeed;
        if (state->console.animationProgress > 1.0f) {
            state->console.animationProgress = 1.0f;
        }
    }
    else if (!state->console.active && state->console.animationProgress > 0.0f) {
        state->console.animationProgress -= state->console.animationSpeed;
        if (state->console.animationProgress < 0.0f) {
            state->console.animationProgress = 0.0f;
        }
    }

    static bool32 consolePressedReg = 0;
    if (input->buttons & BUTTON_CONSOLE) {
        if (!consolePressedReg) {
            state->console.active = !state->console.active;
            consolePressedReg = 1;
        }
    }
    else {
        consolePressedReg = 0;
    }

}

void DrawElapsedTime(GameState *state, RenderContext *ctx, v2i windowSize, r32 frameTime) {

    char *output = mprintf("%.2f seconds elapsed.", state->elapsedTime);
    SCOPE_FREE(output);
    DrawText(ctx, output, { windowSize.x - 300 , 0 }, Color(1.0f, 1.0f, 1.0f));

    char *lastFrame = mprintf("Frame time: %.2f", frameTime);
    SCOPE_FREE(lastFrame);
    DrawText(ctx, lastFrame, { windowSize.x - 300, 18}, Color(1.0f, 1.0f, 1.0f));
}

extern "C"
void EXPORT UpdateGame(PlatformState *platformState, GameMemory *memory, GameInput *input, r32 dt) {

    GameState *state = (GameState *)memory->permanent;    
    if (!state->initialized) {
        GameInit(state, platformState->api, memory);
    }

    // TODO: Get rid of this ad-hoc loading of glew function pointers.
    if (platformState->libReloaded) {
        glewInit();
        WriteConsole(&state->console, "Loading game library.");
        platformState->libReloaded = false;
    }

    state->elapsedTime += dt;

    state->world->screenSize = platformState->windowSize;

    // Updating
    UpdateConsole(state, input);
    WorldUpdate(state->world, input, dt);

    // Rendering
    RenderStart(state->renderer, platformState->windowSize);
    WorldRender(state->world, state->renderer, platformState->windowSize);

    DrawConsole(&state->console, state->renderer, state->world->screenSize);

    DrawElapsedTime(state, state->renderer, platformState->windowSize, platformState->lastFrameTime);

    RenderEnd(state->renderer);

    if (state->shutdown) {
        platformState->shutdownRequested = true;
    }
}


