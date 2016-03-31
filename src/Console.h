#ifndef _CONSOLE_H
#define _CONSOLE_H

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

#endif

