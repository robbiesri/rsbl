// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#pragma once

#include <rsbl-math-types.h>
#include <rsbl-ptr.h>
#include <rsbl-result.h>

// TODO: capture input state
// TODO: Handle Window callback (for imgui stuff)
// TODO: Cursor management?
// TODO: Explicit message pumping?
// TODO: handle fullscreen, esp relevant for mobile
// TODO: Surface the window and client regions thru Window API?

namespace rsbl
{

// Opaque handle to platform-specific window data
struct WindowNativeData
{
    void* platform_handle;
};

class Window
{
  public:
    // Factory method to create a window
    static Result<UniquePtr<Window>> Create(uint2 size, int2 position = {-1, -1});

    // Destructor
    ~Window();

    // Moveable, not copyable
    Window(Window&&) noexcept = default;
    Window& operator=(Window&&) noexcept = default;
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    WindowNativeData GetNativeData() const;

    // Window management methods
    void Show();
    void Hide();
    bool IsVisible() const;

    // Dimension and position accessors
    uint32 Width() const
    {
        return m_size.x;
    }
    uint32 Height() const
    {
        return m_size.y;
    }
    int32 X() const
    {
        return m_position.x;
    }
    int32 Y() const
    {
        return m_position.y;
    }

    uint2 Size() const
    {
        return m_size;
    }
    int2 Position() const
    {
        return m_position;
    }

    // Unfortunate to leak some platform-specific bits into header, but this is simpler than doing
    // some PIMPL thing
#ifdef _WIN32
    // Platform-specific window procedure
    static long long WindowProc(void* handle,
                                unsigned int uMsg,
                                unsigned long long wParam,
                                long long lParam);

#endif

  protected:
    // Private constructor - use Create() factory method
    Window(uint2 size, int2 position);
    uint2 m_size;
    int2 m_position;

    // Platform-specific implementation data
    WindowNativeData m_platformData;
};

} // namespace rsbl
