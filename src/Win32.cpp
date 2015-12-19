#include <Windows.h>

LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LRESULT result;
    switch (uMsg) {
        default:
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return result;
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

    bool running = true;
    while(running) {

        // Main game loop:
        // - Handle input
        // - Update game state
        // - Render
        // - Delay to keep 60 fps

        MSG msg;
        while (PeekMessage(&msg, window, NULL, NULL, PM_REMOVE)) {
            // Handle input.
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return EXIT_SUCCESS;
}

