// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include "rsbl-window.h"

#include <windows.h>

// TODO: Allocate space for a window pointer in WNDCLASSEXA.cbWndExtra
// TODO: Set a pointer to the window impl class via SetWindowLongPtrA
// TODO: Should I consider using CS_CLASSDC or CS_OWNDC in WNDCLASSEXA::style? I'm always confused
// how it affects DX/Vulkan API interactions
// TODO: use AdjustWindowRect to correct rect based on position, size and actual display properties
// TODO: Handle resizing (WM_SIZE?)
// TODO: hook into imgui window management
// TODO: Query and update actual window position + size from API

namespace rsbl
{

// Window class name for registration
static const char* WINDOW_CLASS_NAME = "RSBLWindowClass";
static bool s_windowClassRegistered = false;

// Forward declaration for window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Register the window class (only needs to happen once)
static Result<> RegisterWindowClass()
{
    if (s_windowClassRegistered)
    {
        return ResultCode::Success;
    }

    WNDCLASSEXA wc;
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hIcon = nullptr;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    // lol, converting from color types to HBRUSH, we need to offset by 1. Insanity
    // See: https://www.gamedev.net/forums/topic/440835-what-is-color_window/?page=2
    // I could remove this, but I'll keep it to fill in the 'dead' area on expanding resize
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hIconSm = nullptr;

    if (!RegisterClassExA(&wc))
    {
        return "Failed to register window class";
    }

    s_windowClassRegistered = true;
    return ResultCode::Success;
}

// We can expect WM_CLOSE -> WM_DESTROY -> WM_QUIT. Some of it is explained here:
// https://stackoverflow.com/questions/3155782/what-is-the-difference-between-wm-quit-wm-close-and-wm-destroy-in-a-windows-pr
// I'm just going to handle DESTROY, and let windows handle CLOSE and QUIT (though I'm kinda
// invoking QUIT by caling PostQuitMessage).
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
}

Result<Window*> Window::Create(uint32 width, uint32 height, int32 x, int32 y)
{
    // Ensure window class is registered
    if (auto registerResult = RegisterWindowClass(); registerResult.Code() != ResultCode::Success)
    {
        return "Failed to register window class";
    }

    // If position is -1, use default positioning (CW_USEDEFAULT)
    const int posX = (x == -1) ? CW_USEDEFAULT : x;
    const int posY = (y == -1) ? CW_USEDEFAULT : y;

    // Create the window (initially hidden)
    const HWND hwnd = CreateWindowExA(0,                        // Extended window style
                                      WINDOW_CLASS_NAME,        // Window class name
                                      "RSBL Window",            // Window title
                                      WS_OVERLAPPEDWINDOW,      // Window style
                                      posX,                     // X position
                                      posY,                     // Y position
                                      static_cast<int>(width),  // Width
                                      static_cast<int>(height), // Height
                                      nullptr,                  // Parent window
                                      nullptr,                  // Menu
                                      GetModuleHandle(nullptr), // Instance
                                      nullptr);                 // Additional data

    if (hwnd == nullptr)
    {
        return "Failed to create window";
    }

    // Allocate and initialize the Window object
    Window* window = new Window(width, height, x, y);
    window->m_platformData.platform_handle = hwnd;

    window->Show();

    return window;
}

Window::Window(uint32 width, uint32 height, int32 x, int32 y)
    : m_width(width)
    , m_height(height)
    , m_x(x)
    , m_y(y)
    , m_platformData{nullptr}
{
}

Window::~Window()
{
    if (m_platformData.platform_handle != nullptr)
    {
        HWND hwnd = static_cast<HWND>(m_platformData.platform_handle);
        DestroyWindow(hwnd);
        m_platformData.platform_handle = nullptr;
    }
}

WindowNativeData Window::GetNativeData() const
{
    return m_platformData;
}

void Window::Show()
{
    if (m_platformData.platform_handle != nullptr)
    {
        HWND hwnd = static_cast<HWND>(m_platformData.platform_handle);
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }
}

void Window::Hide()
{
    if (m_platformData.platform_handle != nullptr)
    {
        HWND hwnd = static_cast<HWND>(m_platformData.platform_handle);
        ShowWindow(hwnd, SW_HIDE);
    }
}

bool Window::IsVisible() const
{
    if (m_platformData.platform_handle != nullptr)
    {
        HWND hwnd = static_cast<HWND>(m_platformData.platform_handle);
        return IsWindowVisible(hwnd) != 0;
    }
    return false;
}

} // namespace rsbl
