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

typedef struct {
    bitmap content;
    BITMAPINFO info;
} backbuffer;

static backbuffer buffer = {};

// TODO: Move to program state.
static bool isRunning;
static bool isInitialized = 0;


void ResizeWindow(uint32 width, uint32 height) {
   if (buffer.content.data) {
       VirtualFree(buffer.content.data, 0, MEM_RELEASE);
   }
   
   buffer.info = {};
   buffer.info.bmiHeader.biSize = sizeof(buffer.info.bmiHeader);
   buffer.info.bmiHeader.biWidth = (LONG)width;
   buffer.info.bmiHeader.biHeight = -(LONG)height;
   buffer.info.bmiHeader.biPlanes = 1;
   buffer.info.bmiHeader.biBitCount = 32;
   buffer.info.bmiHeader.biCompression = BI_RGB;

   buffer.content.data = VirtualAlloc(0, 32 * width * height, MEM_COMMIT, PAGE_READWRITE);


   glViewport(0, 0, width, height);
}

void RenderBackbuffer(HWND hwnd, backbuffer *buff) {
    HDC target = GetDC(hwnd);

    RECT targetRect;
    GetClientRect(hwnd, &targetRect);

    int32 width = targetRect.right - targetRect.left;
    int32 height = targetRect.bottom - targetRect.top;

    //StretchDIBits(target, 0, 0, width, height, 0, 0, width, height, buff->content.data, &buff->info, DIB_RGB_COLORS, SRCCOPY);

    SwapBuffers(target);

    ReleaseDC(hwnd, target);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;
    switch (uMsg) {
        case WM_CLOSE:{
                          isRunning = false;
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

    isRunning = true;
    while(isRunning) {

        // Main game loop:
        // - Handle input
        // - Update game state
        // - Render
        
        QueryPerformanceCounter(&tStart);
        gameLib.UpdateGame(&api, &memory, NULL);
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
    ReleaseDC(window, hdc);

    return EXIT_SUCCESS;
}

