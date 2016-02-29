#include "Game.h"
#include "Platform.h"
#include "Math.h"
#include "Renderer.h"
#include "World.h"

typedef struct {
    bool initialized;
    RenderContext *renderer;
    MemorySegment game_memory;
    MemorySegment renderer_memory;
    GameAssets assets;
    World *world;

    bool showConsole;
    char consoleInput[256];
    u32 consoelInputCount;
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

    state->showConsole = false;
    state->initialized = true;
}

void DrawConsole(game_state *state, RenderContext *ctx) {
    Rect2Di targetRect = { 0, 0, state->world->screenSize.x, state->world->screenSize.y / 2 };
    Color bgcolor = { .25f, .35f, .35f };
    DrawSolidRect(ctx, targetRect, bgcolor);

    DrawText(ctx, state->consoleInput, { 0, state->world->screenSize.y / 2 - 20 }, { 1.0f, 1.0f, 1.0f });
}

void ReadConsoleInput(game_state *state, keyboard_state *keyboard) {
    for (i32 i = 0; i < keyboard->keyCount; ++i) {
        if (keyboard->keyStack[i] >= 'a' && keyboard->keyStack[i] <= 'z') {
            state->consoleInput[state->consoelInputCount++] = keyboard->keyStack[i];
        }
        // TODO: Backspace, caret position, etc...
    }
    state->consoleInput[state->consoelInputCount + 1] = 0;
}

extern "C"
void EXPORT UpdateGame(platform_state *platformState, game_memory *memory, game_input *input, real32 dt) {

    game_state *state = (game_state *)memory->permanent;    
    if (!state->initialized) {
        GameInit(state, platformState->api, memory);
    }

    state->world->screenSize = platformState->windowSize;

    static bool32 consolePressedReg = 0;
    if (input->buttons & BUTTON_CONSOLE) {
        if (!consolePressedReg) {
            state->showConsole = !state->showConsole;
            consolePressedReg = 1;
        }
    } else {
        consolePressedReg = 0;
    }

    // Updating
    if (state->showConsole) {
        ReadConsoleInput(state, &input->keyboard);
    }
    WorldUpdate(state->world, input, dt);

    // Rendering
    RenderStart(state->renderer, platformState->windowSize);
    WorldRender(state->world, state->renderer, platformState->windowSize);

    if (state->showConsole) {
        DrawConsole(state, state->renderer);
    }

    RenderEnd(state->renderer);
}


