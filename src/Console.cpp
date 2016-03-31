#include "Console.h"
#include "Game.h"

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
        state->shutdown = 1;
    } else if (strcmp("reload", buff) == 0) {
        WriteConsole(&state->console, "Reloading...");
        state->restart = 1;
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
    Color bgcolor = { .25f, .35f, .35f, 0.9f };
    DrawSolidRect(ctx, targetRect, bgcolor);
    
    // Draw input in buffer
    i32 index = console->logNext - 1;
    for (i32 i = 0; i < CONSOLE_LOG_SIZE; ++i) {
        // TODO: Dont bother drawing outside screen...
        if (index < 0) {
            index = CONSOLE_LOG_SIZE - 1;
        }

        DrawText(ctx, console->log[index], { 0, height - 40 - 20 * i }, { 1.0f, 1.0f, 1.0f });
        index--;
    }


    Color inputbg = { 0.2f, 0.2f, 0.2f };
    DrawSolidRect(ctx, { 1, height - 19, width - 2, 18}, inputbg);
    DrawText(ctx, ">", { 2, height - 18 }, { 1.0f, 1.0f, 1.0f });
    DrawText(ctx, console->input, { 20, height - 18 }, { 1.0f, 1.0f, 1.0f });
}

void ReadConsoleInput(GameState *state, ConsoleState *console, KeyboardState *keyboard) {
    
    if (!console->active) {
        // Console is not displayed, do not consume input
        return;
    }

    for (i32 i = 0; i < keyboard->keyCount; ++i) {
        u32 key = keyboard->keyStack[i];
        if (key == '\b') {
            // Consume the backspace input.
            if (console->inputCount > 0) {
                console->inputCount--;
                console->input[console->inputCount] = 0;
            }
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


