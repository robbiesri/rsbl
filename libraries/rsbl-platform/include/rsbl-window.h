// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#pragma once

#include <rsbl-int-types.h>
#include <rsbl-result.h>

// TODO: capture input state
// TODO: Handle Window callback (for imgui stuff)
// TODO: Cursor management?
// TODO: Explicit message pumping?
// TODO: vector types like uint2
// TODO: handle fullscreen, esp relevant for mobile

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
    static Result<Window*> Create(uint32 width, uint32 height, int32 x = -1, int32 y = -1);

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
        return m_width;
    }
    uint32 Height() const
    {
        return m_height;
    }
    int32 X() const
    {
        return m_x;
    }
    int32 Y() const
    {
        return m_y;
    }

  private:
    // Private constructor - use Create() factory method
    Window(uint32 width, uint32 height, int32 x, int32 y);
    uint32 m_width;
    uint32 m_height;
    int32 m_x;
    int32 m_y;

    // Platform-specific implementation data
    WindowNativeData m_platformData;
};

} // namespace rsbl
