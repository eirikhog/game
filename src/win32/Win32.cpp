#include "../Platform.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "../../dep/glew-1.13.0/src/glew.c"

// TODO: Dynamic loading
#include <xinput.h>

#include <chrono>
#include <thread>

#ifdef _DEBUG
#undef Assert
#define Assert(x) if(!(x)) { OutputDebugString("Assert failed: " #x "\n"); (*(int*)(0)) = 0; }
#else
#undef Assert
#define Assert(x)
#endif

#ifdef _DEBUG
typedef struct {
    u32 frameCount;
    u64 workTime;
    u64 sleepTime;
} Win32Performance;
#endif

#define GAMELIB "Game.dll"
#define GAMELIB_ACTIVE "Game_loaded.dll"

typedef struct {
    bool running;
    WINDOWPLACEMENT windowPlacement;
    PlatformState *platformState;

    HMODULE gamelibHandle;
    Time gamelibTimestamp;
} Win32State;

void ResizeWindow(PlatformState *platformState, u32 width, u32 height) {
    platformState->windowSize = { (i32)width, (i32)height };
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
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)state_ptr);
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        }break;
        case WM_CLOSE:{
            Win32State *state = (Win32State*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            state->running = false;
        }break;
        case WM_SIZE: {
            Win32State *state = (Win32State*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if (state && state->platformState) {
                RECT windowRect;
                GetClientRect(hwnd, &windowRect);
                ResizeWindow(state->platformState, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top);
            }
        }break;
        default:
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return result;
}

inline i64 GetElapsedMicroseconds(LARGE_INTEGER start, LARGE_INTEGER end, LARGE_INTEGER freq) {
    i64 elapsedMicroseconds = (1000000 * (end.QuadPart - start.QuadPart) / freq.QuadPart);
    return elapsedMicroseconds;
}

extern "C" void OutputDebug(char *text) {
    OutputDebugString(text);
}

char *Win32ReadFile(char *filename, u32 *filesize) {
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

    CloseHandle(handle);

    return (char*)content;
}

void Win32WriteFile(char *filename, void *data, u32 size) {
    HANDLE handle = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    Assert(handle != INVALID_HANDLE_VALUE);
    if (handle == INVALID_HANDLE_VALUE) {
        return;
    }

    DWORD written;
    WriteFile(handle, data, size, &written, NULL);
    Assert(written == size);
    CloseHandle(handle);
}

Time GetLastWriteTime(const char *file) {
    Time result;
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (GetFileAttributesEx(file, GetFileExInfoStandard, &fileInfo)) {
        FILETIME lastWriteFt = fileInfo.ftLastWriteTime;
        SYSTEMTIME lastWrite;
        FileTimeToSystemTime(&lastWriteFt, &lastWrite);

        result.year = lastWrite.wYear;
        result.month = lastWrite.wMonth;
        result.day = lastWrite.wDay;
        result.hour = lastWrite.wHour;
        result.minute = lastWrite.wMinute;
        result.second = lastWrite.wSecond;
        result.millisecond = lastWrite.wMilliseconds;

    } else {
        InvalidCodePath();
    }

    return result;
}

GameFunctions LoadGameLibrary(Win32State *state) {
    GameFunctions library = {};
    library.DebugOutput = OutputDebug;

    u32 copyRetryCount = 5;
    BOOL copyResult;
    do {
        copyResult = CopyFile(GAMELIB, GAMELIB_ACTIVE, false);
        if (!copyResult) {
            Sleep(200);
        }
    } while (!copyResult && --copyRetryCount);

    if (!copyResult) {
        InvalidCodePath();
    }

    state->gamelibHandle = LoadLibrary(GAMELIB_ACTIVE);
    library.UpdateGame = (update_game *)GetProcAddress(state->gamelibHandle, "UpdateGame");
    Assert(library.UpdateGame);
    Assert(state->gamelibHandle);

    return library;
}

void UnloadGameLibrary(Win32State *state) {
    FreeLibrary(state->gamelibHandle);
    state->gamelibHandle = 0;
}


void ToggleFullscreen(HWND window, Win32State *state) {

    DWORD style = GetWindowLong(window, GWL_STYLE);
    if (style & WS_OVERLAPPEDWINDOW) {
        HMONITOR hmon = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { sizeof(MONITORINFO) };
        if (!GetMonitorInfo(hmon, &mi) || !GetWindowPlacement(window, &state->windowPlacement)) {
            return;
        }

        SetWindowLong(window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
        SetWindowPos(window, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                mi.rcMonitor.right - mi.rcMonitor.left,
                mi.rcMonitor.bottom - mi.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    } else {
        SetWindowLong(window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(window, &state->windowPlacement);
        SetWindowPos(window, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
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

void UpdateJoystick(GameInput *input) {
    // TODO: Handle more than one joystick
    XINPUT_STATE state;
    ZeroMemory(&state, sizeof(XINPUT_STATE));

    if (!XInputGetState(0, &state)) {
        v2f leftStick = { (r32)state.Gamepad.sThumbLX, (r32)state.Gamepad.sThumbLY };
        r32 magnitude = sqrt( leftStick.x * leftStick.x + leftStick.y * leftStick.y);
        if (magnitude > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
            v2f normalized = leftStick / magnitude;
            input->joystick = normalized;
        } else {
            input->joystick = { 0.0f, 0.0f };
        }

        if (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) {
            input->buttons |= BUTTON_UP;
        } else {
            input->buttons &= ~BUTTON_UP;
        }
    } else {
        // Did not find controller
    }
}

void HandleKeyInput(GameInput *input, InputButtons button, bool32 pressed) {
    if (pressed) {
        input->buttons |= button;
    } else {
        input->buttons &= ~button;
    }
}

void InputKeyboardReset(KeyboardState *keyboard) {
    keyboard->keyCount = 0;
}

void InputPushKeyboard(KeyboardState *keyboard, u32 key) {
    if (keyboard->keyCount >= KEYBOARD_MAX) {
        return; // Ignore key -- buffer is full
    }

    keyboard->keyStack[keyboard->keyCount++] = key;
}

// Platform API, TODO: Move to separate file?
FileHandle Platform_FileOpen(char *filename) {
    HANDLE hfile = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL,
            CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

    return (FileHandle)hfile;
}

bool32 Platform_FileClose(FileHandle handle) {
    return (bool32)CloseHandle(handle);
}

u32 Platform_FileRead(FileHandle handle, u32 size, void *dest) {
    DWORD bytesRead = 0;
    BOOL result = ReadFile(handle, dest, size, &bytesRead, NULL);
    Assert(result);

    return (u32)bytesRead;
}

u32 Platform_FileWrite(FileHandle handle, u32 size, void *src) {
    DWORD bytesWritten;
    BOOL result = WriteFile(handle, src, size, &bytesWritten, NULL);
    Assert(result);

    return (u32)bytesWritten;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    Win32State programState = {};

    PlatformState platformState = {};
    PlatformAPI api = {};
    api.ReadEntireFile = Win32ReadFile;
    api.WriteEntireFile = Win32WriteFile;
    api.FileOpen = Platform_FileOpen;
    api.FileClose = Platform_FileClose;
    api.FileRead = Platform_FileRead;
    api.FileWrite = Platform_FileWrite;

    platformState.api = &api;
    programState.platformState = &platformState;
    
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
            CW_USEDEFAULT, CW_USEDEFAULT, 1366, 768, NULL, NULL, hInstance, &programState);

    if (!window) {
        OutputDebugString("Could not create window.");
        return EXIT_FAILURE;
    }
    
    LARGE_INTEGER tStart, tEnd;
    LARGE_INTEGER cpuFreq;
    QueryPerformanceFrequency(&cpuFreq);

    // Initialize game functions.
    // TODO: Copy the game dll to a different location, so that the compiler can
    // overwrite the old file with a newer version...
    GameFunctions gameLib = LoadGameLibrary(&programState);
    programState.gamelibTimestamp = GetLastWriteTime(GAMELIB);
    platformState.libReloaded = true;
    GameMemory memory = {};
    
    const u32 permanentMemorySize = 1024 * 1024 * 256;
    memory.permanent = VirtualAlloc(0, permanentMemorySize, MEM_COMMIT, PAGE_READWRITE);
    memory.permanentSize = permanentMemorySize;

    const u32 transientMemorySize = 1024 * 1024 * 256;
    memory.transient = VirtualAlloc(0, transientMemorySize, MEM_COMMIT, PAGE_READWRITE);
    memory.transientSize = transientMemorySize;

    // TODO: Move this 
    HDC hdc = GetDC(window);
    SetupOpenGL(hdc);

    LARGE_INTEGER ftStart, ftEnd;
    QueryPerformanceCounter(&ftStart);
    r32 frameTime = 1.0f / 60.0f;

#ifdef _DEBUG
    Win32Performance perf = {};
#endif
    v2i mouse_prev_position = {};
    HCURSOR mouse_cursor = LoadCursor(NULL, IDC_ARROW);
    GameInput input = {};

    programState.running = true;
    while(programState.running) {

        Time lastWrite = GetLastWriteTime(GAMELIB);
        if (compareTime(&lastWrite, &programState.gamelibTimestamp) > 0) {
            UnloadGameLibrary(&programState);
            gameLib = LoadGameLibrary(&programState);
            programState.gamelibTimestamp = GetLastWriteTime(GAMELIB);
            platformState.libReloaded = true;
        }
        
        QueryPerformanceCounter(&tStart);    
        UpdateJoystick(&input);
        InputKeyboardReset(&input.keyboard);

        MSG msg;
        while (PeekMessage(&msg, window, NULL, NULL, PM_REMOVE)) {
            switch (msg.message) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                case WM_MOUSEMOVE:{
                    SetCursor(mouse_cursor);

                    u32 xPos = 0xFFFF & msg.lParam;
                    u32 yPos = (u64)(0xFFFF0000 & msg.lParam) >> 16;
                    input.mouse_position = { (i32)xPos, (i32)yPos };
                    input.mouse_buttons = 0;
                    if (msg.wParam & MK_LBUTTON) {
                        input.mouse_buttons |= MOUSE_LEFT;
                    }
                    if (msg.wParam & MK_RBUTTON) {
                        input.mouse_buttons |= MOUSE_RIGHT;
                    }
                }break;
                case WM_KEYUP:
                case WM_KEYDOWN:{
                    bool32 wasUp = msg.lParam & (1 << 30);
                    bool32 pressed = !(msg.lParam & (1 << 31));
                    switch (msg.wParam) {
                        case VK_UP:
                            HandleKeyInput(&input, BUTTON_UP, pressed);
                            break;
                        case VK_DOWN:
                            HandleKeyInput(&input, BUTTON_LEFT, pressed);
                            break;
                        case VK_LEFT:
                            HandleKeyInput(&input, BUTTON_LEFT, pressed);
                            break;
                        case VK_RIGHT:
                            HandleKeyInput(&input, BUTTON_RIGHT, pressed);
                            break;
                        case VK_OEM_3:
                            // Tilde key
                            HandleKeyInput(&input, BUTTON_CONSOLE, pressed);
                            break;
                        case VK_F11:
                            if (pressed) {
                                ToggleFullscreen(window, &programState);
                            }
                            break;
                        case VK_ESCAPE:
                            programState.running = false;
                            break;
                    }
                    if (pressed) {
                        // TODO: Consider unicode?
                        UINT vkCode = msg.wParam;
                        UINT scanCode = (msg.lParam & 0x7F0000) >> 16;
                        u8 keyState[256];
                        GetKeyboardState(keyState);
                        wchar_t buffer[8];
                        ZeroMemory(buffer, 8);
                        u32 code = ToUnicode(vkCode, scanCode, keyState, buffer, 8, 0);
                        if (code) {
                            InputPushKeyboard(&input.keyboard, 0xFF & (u32)buffer[0]);
                        }
                    }
                }break;
                default:
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                    break;
            }
        }

        input.mouse_delta = mouse_prev_position - input.mouse_position;
        mouse_prev_position = input.mouse_position;
        gameLib.UpdateGame(&platformState, &memory, &input, frameTime);
        SwapBackbuffer(window);
        
        QueryPerformanceCounter(&tEnd); // TODO: Replace with std::chrono? Do testing...
        u64 tElapsedUs = GetElapsedMicroseconds(tStart, tEnd, cpuFreq);
#ifdef _DEBUG
        perf.workTime += tElapsedUs;
#endif
        const int64_t targetTime = 1000000 / 60; // us in 1 frame at 60 fps
        int64_t sleepTime = (targetTime - tElapsedUs);
#ifdef _DEBUG
        perf.sleepTime += sleepTime;
#endif
        if (sleepTime > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(sleepTime));
            frameTime = 1.0f / 60.0f;
        } else {
            // If the process did not sleep, it means that we have too much work for the frame!
            //frameTime = (r32)tElapsedUs / 1000000.0f;

            // Cap the timesteps to avoid glitches
            frameTime = 1.0f / 60.0f;
        }

        platformState.lastFrameTime = (r32)tElapsedUs / 1000.0f;
        if (platformState.shutdownRequested) {
            programState.running = false;
        }

#ifdef _DEBUG
        perf.frameCount++;
        if (perf.frameCount == 59) {
            QueryPerformanceCounter(&ftEnd);
            u64 a = GetElapsedMicroseconds(ftStart, ftEnd, cpuFreq);
            float time_per_frame = (float)perf.workTime / 60000.0f;
            float sleep_per_frame = (float)perf.sleepTime / 60000.0f;
            float utilization = 100.0f * time_per_frame / (sleep_per_frame + time_per_frame);
            char text[256];
            sprintf_s(text, 256, "Performance: Time per frame: %f ms. Sleep per frame: %f (%f %%)\n", time_per_frame, sleep_per_frame, utilization);
            OutputDebugString(text);
            perf.frameCount = 0;
            perf.workTime = 0;
            perf.sleepTime = 0;
            QueryPerformanceCounter(&ftStart);
        }
#endif

    }
    ReleaseDC(window, hdc);

    return EXIT_SUCCESS;
}

