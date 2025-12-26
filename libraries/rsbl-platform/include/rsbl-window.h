// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#pragma once

#include <rsbl-int-types.h>
#include <rsbl-result.h>

// TODO: capture input state
// TODO: Handle Window callback (for imgui stuff)
// TODO: Cursor management?
// TODO: Explicit message pumping?

namespace rsbl
{

// Opaque handle to platform-specific window data
struct WindowNativeHandle
{
    void* platform_data;
};

class Window
{
  public:
    // Factory method to create a window
    static Result<Window*> Create(uint32 width, uint32 height, int32 x = -1, int32 y = -1);

    // Destructor
    ~Window();

    // Delete copy constructor and copy assignment (not copyable)
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // Default move constructor and move assignment (moveable)
    Window(Window&&) noexcept = default;
    Window& operator=(Window&&) noexcept = default;

    // Get platform-specific window data
    WindowNativeHandle GetNativeHandle() const;

    // Window management methods
    void Show();
    void Hide();
    bool IsVisible() const;

    // Dimension and position accessors
    uint32 GetWidth() const
    {
        return width_;
    }
    uint32 GetHeight() const
    {
        return height_;
    }
    int32 GetX() const
    {
        return x_;
    }
    int32 GetY() const
    {
        return y_;
    }

  private:
    // Private constructor - use Create() factory method
    Window(uint32 width, uint32 height, int32 x, int32 y);
    uint32 width_;
    uint32 height_;
    int32 x_;
    int32 y_;

    // Platform-specific implementation data
    void* platform_window_;
};

} // namespace rsbl
