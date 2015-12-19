#include <Windows.h>
#include <stdint.h>

#include <chrono>
#include <thread>

LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LRESULT result;
    switch (uMsg) {
        default:
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return result;
}

inline int64_t
GetElapsedMicroseconds(LARGE_INTEGER start, LARGE_INTEGER end, LARGE_INTEGER freq) {
    int64_t elapsedMicroseconds = (1000000 * (end.QuadPart - start.QuadPart) / freq.QuadPart);
    return elapsedMicroseconds;
}

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

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

    bool running = true;

    while(running) {

        // Main game loop:
        // - Handle input
        // - Update game state
        // - Render
        // - Delay to keep 60 fps
        

        QueryPerformanceCounter(&tStart);

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

