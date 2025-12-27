// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#pragma once

#include "rsbl-core.h"

namespace rsbl
{

template <typename T>
class UniquePtr
{
  public:
    // Default constructor - creates null pointer
    UniquePtr()
        : m_ptr(nullptr)
    {
    }

    // Constructor from raw pointer - takes ownership
    explicit UniquePtr(T* ptr)
        : m_ptr(ptr)
    {
    }

    // Destructor - deletes managed object
    ~UniquePtr()
    {
        if (m_ptr != nullptr)
        {
            delete m_ptr;
        }
    }

    // Move constructor
    UniquePtr(UniquePtr&& other) noexcept
        : m_ptr(other.m_ptr)
    {
        other.m_ptr = nullptr;
    }

    // Move assignment operator
    UniquePtr& operator=(UniquePtr&& other) noexcept
    {
        if (this != &other)
        {
            // Delete current object
            if (m_ptr != nullptr)
            {
                delete m_ptr;
            }

            // Take ownership from other
            m_ptr = other.m_ptr;
            other.m_ptr = nullptr;
        }
        return *this;
    }

    // Delete copy constructor and copy assignment
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    // Dereference operator
    T& operator*() const
    {
        return *m_ptr;
    }

    // Arrow operator
    T* operator->() const
    {
        return m_ptr;
    }

    // Get raw pointer
    T* Get() const
    {
        return m_ptr;
    }

    // Release ownership without deleting
    T* Release()
    {
        T* ptr = m_ptr;
        m_ptr = nullptr;
        return ptr;
    }

    // Reset to new pointer (deletes current)
    void Reset(T* ptr = nullptr)
    {
        if (m_ptr != nullptr)
        {
            delete m_ptr;
        }
        m_ptr = ptr;
    }

    // Bool conversion operator
    explicit operator bool() const
    {
        return m_ptr != nullptr;
    }

  private:
    T* m_ptr;
};

// Helper function to create UniquePtr with perfect forwarding
template <typename T, typename... Args>
UniquePtr<T> MakeUnique(Args&&... args)
{
    return UniquePtr<T>(new T(rsblForward(args)...));
}

} // namespace rsbl
