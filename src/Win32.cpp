#include "Platform.h"

#include <Windows.h>
#include <GL/glew.h>
#include <GL/wglew.h>

#include <chrono>
#include <thread>
#include <fstream>

using namespace std;

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

// TODO: Move to program state.
static bool isRunning;
static bool isInitialized = 0;

GLfloat vertices[] = { -1.0f,0.0f,0.0f, 0.0f,1.0f,0.0f, 0.0f,0.0f,0.0f };
GLfloat colours[] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat vertices2[] = { 0.0f,0.0f,0.0f, 0.0f,-1.0f,0.0f, 1.0f,0.0f,0.0f };

// two vertex array objects, one for each object drawn
unsigned int vertexArrayObjID[2];
// three vertex buffer objects in this example
unsigned int vertexBufferObjID[3];

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

   uint32 *pixel = (uint32*)buffer.content.data;
   for (uint32 y = 0; y < width; ++y) {
       for (uint32 x = 0; x < height; ++x) {
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

    //StretchDIBits(target, 0, 0, width, height, 0, 0, width, height, buff->content.data, &buff->info, DIB_RGB_COLORS, SRCCOPY);

    glClear(GL_COLOR_BUFFER_BIT);

    if (isInitialized) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, 0, height, width, 0, 1);

        glBindVertexArray(vertexArrayObjID[0]);	// First VAO
        glDrawArrays(GL_TRIANGLES, 0, 3);	// draw first object

        glBindVertexArray(vertexArrayObjID[1]);		// select second VAO
        glVertexAttrib3f((GLuint)1, 1.0, 0.0, 0.0); // set constant color attribute
        glDrawArrays(GL_TRIANGLES, 0, 3);	// draw second object

        glBindVertexArray(0);
    }

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

game_functions LoadGameLibrary() {
    game_functions library = {};
    library.DebugOutput = OutputDebug;

    HMODULE module = LoadLibrary("Game.dll");
    library.UpdateGame = (update_game *)GetProcAddress(module, "UpdateGame");
    Assert(library.UpdateGame);
    Assert(module);

    return library;
}

char *load_file(char *fname, int &fSize) {
    std::ifstream::pos_type size;
    char * memblock;
    string text;

    // file read based on example in cplusplus.com tutorial
    ifstream file(fname, ios::in | ios::binary | ios::ate);
    if (file.is_open())
    {
        size = file.tellg();
        fSize = (GLuint)size;
        memblock = new char[size];
        file.seekg(0, ios::beg);
        file.read(memblock, size);
        file.close();
        text.assign(memblock);
    }
    else
    {
        exit(1);
    }
    return memblock;
}

void InitializeOpenGL() {
    glGenVertexArrays(2, &vertexArrayObjID[0]);
    glBindVertexArray(vertexArrayObjID[0]);
    glGenBuffers(2, vertexBufferObjID);

    // VBO for vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID[0]);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // VBO for color data
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID[1]);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), colours, GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(vertexArrayObjID[1]);
    glGenBuffers(1, &vertexBufferObjID[2]);

    // VBO for vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID[2]);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GL_FLOAT), vertices2, GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    isInitialized = true;
}

void PrepareOpenGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    GLuint v = glCreateShader(GL_VERTEX_SHADER);
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);

    char *vs, *fs;

    // load shaders & get length of each
    GLint vlen;
    GLint flen;
    vs = load_file("../data/shaders/minimal.vert", vlen);
    fs = load_file("../data/shaders/minimal.frag", flen);

    const char * vv = vs;
    const char * ff = fs;
    glShaderSource(v, 1, &vv, &vlen);
    glShaderSource(f, 1, &ff, &flen);
    GLint compiled;

    glCompileShader(v);
    glGetShaderiv(v, GL_COMPILE_STATUS, &compiled);
    Assert(compiled);

    glCompileShader(f);
    glGetShaderiv(f, GL_COMPILE_STATUS, &compiled);
    Assert(compiled);

    GLuint p = glCreateProgram();
    glBindAttribLocation(p, 0, "in_Position");
    glBindAttribLocation(p, 1, "in_Color");

    glAttachShader(p, v);
    glAttachShader(p, f);

    glLinkProgram(p);
    glUseProgram(p);

    delete[] vs; // dont forget to free allocated memory
    delete[] fs; // we allocated this in the loadFile function...
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

    PrepareOpenGL();
    InitializeOpenGL();
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
    ZeroMemory(memory.transient, memory.transientSize);
    // TODO: Load permanent memory from file?
    
    // TODO: Move this 
    HDC hdc = GetDC(window);
    SetupOpenGL(hdc);
    ReleaseDC(window, hdc);

    isRunning = true;
    while(isRunning) {

        // Main game loop:
        // - Handle input
        // - Update game state
        // - Render
        
        QueryPerformanceCounter(&tStart);
        gameLib.UpdateGame(NULL, NULL, NULL);
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

