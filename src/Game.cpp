#include "Game.h"
#include "Platform.h"
#include "Math.h"
#include "Renderer.h"
#include "World.h"

struct ConsoleState {
    bool32 active;
    char input[256];
    u32 inputCount;

    real32 animationProgress;
    real32 animationSpeed;
};

typedef struct {
    bool initialized;
    RenderContext *renderer;
    MemorySegment game_memory;
    MemorySegment renderer_memory;
    GameAssets assets;
    World *world;

    ConsoleState console;
} game_state;

static void
GameInit(game_state *state, platform_api *api, game_memory *memory) {
    MemorySegment mem_all = {};
    mem_all.size = memory->permanentSize - sizeof(game_state);
    mem_all.base = (uint8*)memory->permanent + sizeof(game_state);
    mem_all.used = 0;
    state->game_memory = mem_all;

    MemorySegment mem_transient = {};
    mem_transient.size = memory->transientSize;
    mem_transient.base = (uint8*)memory->transient;
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

    state->initialized = true;
}

void DrawConsole(ConsoleState *console, RenderContext *ctx, v2i screenSize) {

    if (console->animationProgress <= 0.0f) {
        return;
    }

    i32 width = screenSize.x;
    i32 height = (i32)(sin(console->animationProgress * 3.1415f / 2.0f) * screenSize.y / 2);

    Rect2Di targetRect = { 0, 0, width, height  };
    Color bgcolor = { .25f, .35f, .35f };
    DrawSolidRect(ctx, targetRect, bgcolor);

    Color inputbg = { 0.2f, 0.2f, 0.2f };
    DrawSolidRect(ctx, { 1, height - 19, width - 2, 18}, inputbg);
    DrawText(ctx, ">", { 2, height - 18 }, { 1.0f, 1.0f, 1.0f });
    DrawText(ctx, console->input, { 20, height - 18 }, { 1.0f, 1.0f, 1.0f });
}

void ReadConsoleInput(ConsoleState *console, keyboard_state *keyboard) {
    
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
            // TODO: Execute command
            console->inputCount = 0;
            console->input[0] = 0;
        } else if (key == '`') {
            // Ignore show/hide console command
        }
        else {
            console->input[console->inputCount++] = key;
        }
    }

    console->input[console->inputCount + 1] = 0;
}

void UpdateConsole(game_state *state, game_input *input) {
    ReadConsoleInput(&state->console, &input->keyboard);

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

extern "C"
void EXPORT UpdateGame(platform_state *platformState, game_memory *memory, game_input *input, real32 dt) {

    game_state *state = (game_state *)memory->permanent;    
    if (!state->initialized) {
        GameInit(state, platformState->api, memory);
    }

    state->world->screenSize = platformState->windowSize;

    // Updating
    UpdateConsole(state, input);
    WorldUpdate(state->world, input, dt);

    // Rendering
    RenderStart(state->renderer, platformState->windowSize);
    WorldRender(state->world, state->renderer, platformState->windowSize);

    DrawConsole(&state->console, state->renderer, state->world->screenSize);

    RenderEnd(state->renderer);
}


