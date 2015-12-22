#include "../Platform.h"

#include <Windows.h>
#include <GL/glew.h>
#include <GL/wglew.h>

#include <chrono>
#include <thread>

// TODO: Unify this with platform.h
#ifdef _DEBUG
#undef Assert
#define Assert(x) if(!(x)) { OutputDebugString("Assert failed: " #x "\n"); (*(int*)(0)) = 0; }
#else
#undef Assert
#define Assert(x)
#endif

#ifdef _DEBUG
typedef struct {
    uint32 frame_count;
    uint64 work_time;
    uint64 sleep_time;
} win32_performance;
#endif

typedef struct {
    bool running;
} win32_state;

void ResizeWindow(uint32 width, uint32 height) {
   glViewport(0, 0, width, height);
}

void SwapBackbuffer(HWND hwnd) {
    HDC target = GetDC(hwnd);
    SwapBuffers(target);
    ReleaseDC(hwnd, target);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;
    switch (uMsg) {
        case WM_NCCREATE:{
            // Set game state.
            void* state_ptr = (((LPCREATESTRUCT)lParam)->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG)state_ptr);
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        }break;
        case WM_CLOSE:{
            win32_state *state = (win32_state*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            state->running = false;
        }break;
        case WM_SIZE:{
            RECT windowRect;
            GetClientRect(hwnd, &windowRect);
            ResizeWindow(windowRect.right - windowRect.left, windowRect.bottom - windowRect.top);
        }break;
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

// TODO: We do not want to use this function later... The asset
// system should handle all resources.
char *load_file(char *filename, uint32 *filesize) {
    HANDLE handle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    Assert(handle != INVALID_HANDLE_VALUE);
    if (handle == INVALID_HANDLE_VALUE) {
        filesize = 0;
        return 0;
    }

    DWORD size = GetFileSize(handle, NULL);
    void *content = VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);

    ReadFile(handle, content, size, NULL, NULL);
    *filesize = size;

    return (char*)content;
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

void SetupOpenGL(HDC hdc) {
    
    // TODO: Error handling.
    
    // Create OpenGL Context
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 32;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    Assert(pixelFormat);
    BOOL setPixelResult = SetPixelFormat(hdc, pixelFormat, &pfd);
    Assert(setPixelResult);

    HGLRC tempContext = wglCreateContext(hdc);
    wglMakeCurrent(hdc, tempContext);
    
    GLenum glresult = glewInit();
    Assert(glresult == GLEW_OK);
   
    int openglversion[2];
    glGetIntegerv(GL_MAJOR_VERSION, &openglversion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &openglversion[1]);

    int attribs[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 1,
        WGL_CONTEXT_FLAGS_ARB, 0,
        0
    };

    if (wglewIsSupported("WGL_ARB_create_context") == 1) {
        HGLRC myContext = wglCreateContextAttribsARB(hdc, 0, attribs);
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(tempContext);
        wglMakeCurrent(hdc, myContext);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    win32_state program_state = {};
    
    WNDCLASS myclass = {};
    myclass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
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
            CW_USEDEFAULT, CW_USEDEFAULT, 1366, 768, NULL, NULL, hInstance, &program_state);

    if (!window) {
        OutputDebugString("Could not create window.");
        return EXIT_FAILURE;
    }
    
    LARGE_INTEGER tStart, tEnd;
    LARGE_INTEGER cpuFreq;
    QueryPerformanceFrequency(&cpuFreq);

    // Initialize game functions.
    game_functions gameLib = LoadGameLibrary();
    game_memory memory = {};
    const uint32 transientMemorySize = 1024 * 1024 * 32;
    memory.transient = VirtualAlloc(0, transientMemorySize, MEM_COMMIT, PAGE_READWRITE);
    memory.transientSize = transientMemorySize;
    const uint32 permanentMemorySize = 1024 * 1024 * 16;
    memory.permanent = VirtualAlloc(0, permanentMemorySize, MEM_COMMIT, PAGE_READWRITE);
    memory.permanentSize = permanentMemorySize;
    // TODO: Move this 
    HDC hdc = GetDC(window);
    SetupOpenGL(hdc);

    platform_api api = {};
    api.ReadEntireFile = load_file;

    LARGE_INTEGER ftStart, ftEnd;
    QueryPerformanceCounter(&ftStart);

    win32_performance perf = {};

    program_state.running = true;
    while(program_state.running) {

        // Main game loop:
        // - Handle input
        // - Update game state
        // - Render
        
        QueryPerformanceCounter(&tStart);
        gameLib.UpdateGame(&api, &memory, NULL);
        SwapBackbuffer(window);

        MSG msg;
        while (PeekMessage(&msg, window, NULL, NULL, PM_REMOVE)) {
            // Handle input.
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        QueryPerformanceCounter(&tEnd); // TODO: Replace with std::chrono? Do testing...
        uint64_t tElapsedUs = GetElapsedMicroseconds(tStart, tEnd, cpuFreq);
        perf.work_time += tElapsedUs;
        const int64_t targetTime = 1000000 / 60; // us in 1 frame at 60 fps
        int64_t sleepTime = (targetTime - tElapsedUs);
        perf.sleep_time += sleepTime;
        if (sleepTime > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(sleepTime));
        }

#ifdef _DEBUG
        perf.frame_count++;
        if (perf.frame_count == 59) {
            QueryPerformanceCounter(&ftEnd);
            uint64 a = GetElapsedMicroseconds(ftStart, ftEnd, cpuFreq);
            float time_per_frame = (float)perf.work_time / 60000.0f;
            float sleep_per_frame = (float)perf.sleep_time / 60000.0f;
            float utilization = 100.0f * time_per_frame / (sleep_per_frame + time_per_frame);
            char text[256];
            sprintf_s(text, 256, "Performance: Time per frame: %f ms. Sleep per frame: %f (%f %%)\n", time_per_frame, sleep_per_frame, utilization);
            OutputDebugString(text);
            perf.frame_count = 0;
            perf.work_time = 0;
            perf.sleep_time = 0;
            QueryPerformanceCounter(&ftStart);
        }
#endif

    }
    ReleaseDC(window, hdc);

    return EXIT_SUCCESS;
}

