#include "Platform.h"

#include <Windows.h>

#include <chrono>
#include <thread>

#define DEBUG

// TODO: Unify this with platform.h
#ifdef DEBUG
#undef Assert
#define Assert(x) if(!(x)) { OutputDebugString("Assert failed: " #x "\n"); (*(int*)(0)) = 0; }
#else
#undef Assert
#define Assert(x)
#endif

typedef struct {
    bitmap content;
    BITMAPINFO info;
} backbuffer;

static backbuffer buffer = {};

void ResizeWindow(uint32 width, uint32 height) {
   if (buffer.content.data) {
       VirtualFree(buffer.content.data, 0, MEM_RELEASE);
   }
   
   buffer.info = {};
   buffer.info.bmiHeader.biSize = sizeof(buffer.info.bmiHeader);
   buffer.info.bmiHeader.biWidth = width;
   buffer.info.bmiHeader.biHeight = -height;
   buffer.info.bmiHeader.biPlanes = 1;
   buffer.info.bmiHeader.biBitCount = 32;
   buffer.info.bmiHeader.biCompression = BI_RGB;

   buffer.content.data = VirtualAlloc(0, 32 * width * height, MEM_COMMIT, PAGE_READWRITE);

   uint32 *pixel = (uint32*)buffer.content.data;
   for (int y = 0; y < width; ++y) {
       for (int x = 0; x < height; ++x) {
           *pixel = 0x00FF00FF;
           pixel++;
           
       }
   }
}

void RenderBackbuffer(HWND hwnd, backbuffer *buff) {
    HDC target = GetDC(hwnd);

    RECT targetRect;
    GetClientRect(hwnd, &targetRect);

    int32 width = targetRect.right - targetRect.left;
    int32 height = targetRect.bottom - targetRect.top;

    StretchDIBits(target, 0, 0, width, height, 0, 0, width, height, buff->content.data, &buff->info, DIB_RGB_COLORS, SRCCOPY);

    ReleaseDC(hwnd, target);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LRESULT result;
    switch (uMsg) {
        case WM_SIZE:{
            RECT windowRect;
            GetClientRect(hwnd, &windowRect);
            ResizeWindow(windowRect.right - windowRect.left, windowRect.bottom - windowRect.top);
        }
        default:
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return result;
}

inline int64_t GetElapsedMicroseconds(LARGE_INTEGER start, LARGE_INTEGER end, LARGE_INTEGER freq) {
    int64_t elapsedMicroseconds = (1000000 * (end.QuadPart - start.QuadPart) / freq.QuadPart);
    return elapsedMicroseconds;
}

extern "C" void OutputDebug(char *text) {
    OutputDebugString(text);
}

game_functions LoadGameLibrary() {
    game_functions library = {};
    library.DebugOutput = OutputDebug;

    HMODULE module = LoadLibrary("Game.dll");
    library.UpdateGame = (update_game *)GetProcAddress(module, "UpdateGame");
    Assert(library.UpdateGame);
    Assert(module);

    return library;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS myclass = {};
    myclass.style = CS_HREDRAW | CS_VREDRAW;
    myclass.lpfnWndProc = WindowProc;
    myclass.hInstance = hInstance;
    myclass.hIcon = NULL;
    myclass.hCursor = NULL;
    myclass.hbrBackground = NULL;
    myclass.lpszMenuName = NULL;
    myclass.lpszClassName = "MyGameClass"; 

    if (!RegisterClass(&myclass)) {
        OutputDebugString("Could not register class.");
        return EXIT_FAILURE;
    }

    HWND window = CreateWindow(myclass.lpszClassName, "Game Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, 1366, 768, NULL, NULL, hInstance, NULL);

    if (!window) {
        OutputDebugString("Could not create window.");
        return EXIT_FAILURE;
    }
    
    LARGE_INTEGER tStart, tEnd;
    LARGE_INTEGER cpuFreq;
    QueryPerformanceFrequency(&cpuFreq);

    // Initialize game functions.
    game_functions gameLib = LoadGameLibrary();
    
    bool running = true;
    while(running) {

        // Main game loop:
        // - Handle input
        // - Update game state
        // - Render
        
        QueryPerformanceCounter(&tStart);
        gameLib.UpdateGame(0.f);
        RenderBackbuffer(window, &buffer);

        MSG msg;
        while (PeekMessage(&msg, window, NULL, NULL, PM_REMOVE)) {
            // Handle input.
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        QueryPerformanceCounter(&tEnd); // TODO: Replace with std::chrono? Do testing...
        uint64_t tElapsedUs = GetElapsedMicroseconds(tStart, tEnd, cpuFreq);
        const int64_t targetTime = 15666; // us in 1 frame at 60 fps
        int64_t sleepTime = (targetTime - tElapsedUs);
        if (sleepTime > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(sleepTime));
        }
    }

    return EXIT_SUCCESS;
}

