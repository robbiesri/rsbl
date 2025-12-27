// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include "rsbl-window.h"

#include <windows.h>

// TODO: Should I consider using CS_CLASSDC or CS_OWNDC in WNDCLASSEXA::style? I'm always confused
// how it affects DX/Vulkan API interactions
// TODO: Handle resizing (WM_SIZE?)
// TODO: hook into imgui window management

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
    wc.cbWndExtra = sizeof(Window*); // Allocate space for Window pointer
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
    // Retrieve the Window pointer stored in the window's user data
    // Before we call SetWindowLongPtrA, this will be nullptr. So make we check if window is valid
    // first!
    Window* window = reinterpret_cast<Window*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));

    switch (uMsg)
    {
    case WM_SIZE:
        // Window has been resized
        // lParam contains LOWORD = new width, HIWORD = new height
        // We acknowledge the event but don't update m_width/m_height here
        // The application can query the actual size as needed
        return 0;

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
    if (auto register_result = RegisterWindowClass(); register_result.Code() != ResultCode::Success)
    {
        return "Failed to register window class";
    }

    // If position is -1, use default positioning (CW_USEDEFAULT)
    const int pos_x = (x == -1) ? CW_USEDEFAULT : x;
    const int pos_y = (y == -1) ? CW_USEDEFAULT : y;

    // Adjust the window size to account for borders, title bar, etc.
    // We want 'width' and 'height' to represent the client area size
    RECT client_to_window_rect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    const DWORD window_style = WS_OVERLAPPEDWINDOW;
    const DWORD window_ex_style = 0;

    if (!AdjustWindowRectEx(&client_to_window_rect, window_style, FALSE, window_ex_style))
    {
        return "Failed to adjust window rectangle";
    }

    const int adjusted_width = client_to_window_rect.right - client_to_window_rect.left;
    const int adjusted_height = client_to_window_rect.bottom - client_to_window_rect.top;

    // Create the window (initially hidden)
    const HWND hwnd = CreateWindowExA(window_ex_style,          // Extended window style
                                      WINDOW_CLASS_NAME,        // Window class name
                                      "RSBL Window",            // Window title
                                      window_style,             // Window style
                                      pos_x,                    // X position
                                      pos_y,                    // Y position
                                      adjusted_width,           // Width
                                      adjusted_height,          // Height
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

    // Store the Window pointer in the window's user data for access in WindowProc
    SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));

    // Query the actual window position and size from Windows, but retain the expected 'client' area
    RECT created_window_rect;
    if (GetWindowRect(hwnd, &created_window_rect))
    {
        window->m_position.x = created_window_rect.left;
        window->m_position.y = created_window_rect.top;
    }
    else
    {
        // TODO: log warning? We can't do much, but failing GetWindowRect sems bad.
    }

    // Now get the correct width + height for the _client_ area of the window, which is what we
    // care about for the renderer
    RECT created_client_rect;
    if (GetClientRect(hwnd, &created_client_rect))
    {
        window->m_size.x = static_cast<uint32>(created_client_rect.right - created_client_rect.left);
        window->m_size.y = static_cast<uint32>(created_client_rect.bottom - created_client_rect.top);
    }

    window->Show();

    return window;
}

Window::Window(uint32 width, uint32 height, int32 x, int32 y)
    : m_size(width, height)
    , m_position(x, y)
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
