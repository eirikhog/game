
#include "Platform.h"

#include <Windows.h>

extern "C"
void EXPORT UpdateGame(float dt) {

    OutputDebugString("Hello from Game.dll!\n");
    // stuff...
}


