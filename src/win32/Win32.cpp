#include "../Platform.h"

#include <Windows.h>
#include <GL/glew.h>
#include <GL/wglew.h>

// TODO: Dynamic loading
#include <xinput.h>

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
    WINDOWPLACEMENT windowPlacement;
    platform_state *platformState;
} win32_state;

void ResizeWindow(platform_state *platformState, uint32 width, uint32 height) {
    platformState->windowSize = { (real32)width, (real32)height };
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
        case WM_SIZE: {
            win32_state *state = (win32_state*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
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

inline int64_t GetElapsedMicroseconds(LARGE_INTEGER start, LARGE_INTEGER end, LARGE_INTEGER freq) {
    int64_t elapsedMicroseconds = (1000000 * (end.QuadPart - start.QuadPart) / freq.QuadPart);
    return elapsedMicroseconds;
}

extern "C" void OutputDebug(char *text) {
    OutputDebugString(text);
}

char *Win32ReadFile(char *filename, uint32 *filesize) {
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

void Win32WriteFile(char *filename, void *data, uint32 size) {
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

game_functions LoadGameLibrary() {
    game_functions library = {};
    library.DebugOutput = OutputDebug;

    HMODULE module = LoadLibrary("Game.dll");
    library.UpdateGame = (update_game *)GetProcAddress(module, "UpdateGame");
    Assert(library.UpdateGame);
    Assert(module);

    return library;
}


void ToggleFullscreen(HWND window, win32_state *state) {

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

void UpdateJoystick(game_input *input) {
    // TODO: Handle more than one joystick
    XINPUT_STATE state;
    ZeroMemory(&state, sizeof(XINPUT_STATE));

    if (!XInputGetState(0, &state)) {
        v2 leftStick = { (real32)state.Gamepad.sThumbLX, (real32)state.Gamepad.sThumbLY };
        real32 magnitude = sqrt( leftStick.x * leftStick.x + leftStick.y * leftStick.y);
        if (magnitude > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
            v2 normalized = leftStick / magnitude;
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

void HandleKeyInput(game_input *input, InputButtons button, bool32 pressed) {
    if (pressed) {
        input->buttons |= button;
    } else {
        input->buttons &= ~button;
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    win32_state program_state = {};

    platform_state platformState = {};
    platform_api api = {};
    api.ReadEntireFile = Win32ReadFile;
    api.WriteEntireFile = Win32WriteFile;

    platformState.api = &api;
    program_state.platformState = &platformState;
    
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
    
    const uint32 permanentMemorySize = 1024 * 1024 * 256;
    memory.permanent = VirtualAlloc(0, permanentMemorySize, MEM_COMMIT, PAGE_READWRITE);
    memory.permanentSize = permanentMemorySize;

    const uint32 transientMemorySize = 1024 * 1024 * 32;
    memory.transient = VirtualAlloc(0, transientMemorySize, MEM_COMMIT, PAGE_READWRITE);
    memory.transientSize = transientMemorySize;

    // TODO: Move this 
    HDC hdc = GetDC(window);
    SetupOpenGL(hdc);

    LARGE_INTEGER ftStart, ftEnd;
    QueryPerformanceCounter(&ftStart);
    real32 frameTime = 1.0f / 60.0f;

#ifdef _DEBUG
    win32_performance perf = {};
#endif
    v2 mouse_prev_position = {};
    HCURSOR mouse_cursor = LoadCursor(NULL, IDC_ARROW);
    game_input input = {};

    program_state.running = true;
    while(program_state.running) {
        
        QueryPerformanceCounter(&tStart);    
        UpdateJoystick(&input);

        MSG msg;
        while (PeekMessage(&msg, window, NULL, NULL, PM_REMOVE)) {
            switch (msg.message) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                case WM_MOUSEMOVE:{
                    SetCursor(mouse_cursor);

                    uint32 xPos = 0xFFFF & msg.lParam;
                    uint32 yPos = (uint64)(0xFFFF0000 & msg.lParam) >> 16;
                    input.mouse_position = { (real32)xPos, (real32)yPos };
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
                        case VK_F11:
                            if (pressed) {
                                ToggleFullscreen(window, &program_state);
                            }
                            break;
                        case VK_ESCAPE:
                            program_state.running = false;
                            break;
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
        uint64_t tElapsedUs = GetElapsedMicroseconds(tStart, tEnd, cpuFreq);
#ifdef _DEBUG
        perf.work_time += tElapsedUs;
#endif
        const int64_t targetTime = 1000000 / 60; // us in 1 frame at 60 fps
        int64_t sleepTime = (targetTime - tElapsedUs);
#ifdef _DEBUG
        perf.sleep_time += sleepTime;
#endif
        if (sleepTime > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(sleepTime));
            frameTime = 1.0f / 60.0f;
        } else {
            // If the process did not sleep, it means that we have too much work for the frame!
            frameTime = (real32)tElapsedUs / 1000000.0f;
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

