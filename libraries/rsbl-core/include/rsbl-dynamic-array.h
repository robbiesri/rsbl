// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#pragma once

#include "rsbl-core.h"
#include "rsbl-int-types.h"

#include <cstdlib>
#include <cstring>

namespace rsbl
{

// Simple dynamic array similar to std::vector but with simpler interface
// no allocator support, no emplace, no insert/erase at arbitrary positions

// TODO: support emplace

template <typename T>
class DynamicArray
{
  private:
    T* m_data = nullptr;
    uint64 m_size = 0;
    uint64 m_capacity = 0;

    void Grow(uint64 minCapacity)
    {
        uint64 newCapacity = m_capacity == 0 ? 8 : m_capacity * 2;
        while (newCapacity < minCapacity)
        {
            newCapacity *= 2;
        }

        T* newData = static_cast<T*>(malloc(newCapacity * sizeof(T)));

        // Move existing elements to new buffer
        if (m_data != nullptr)
        {
            for (uint64 i = 0; i < m_size; ++i)
            {
                new (&newData[i]) T(rsblMove(m_data[i]));
                m_data[i].~T();
            }
            free(m_data);
        }

        m_data = newData;
        m_capacity = newCapacity;
    }

  public:
    DynamicArray() = default;

    // Constructor with initial capacity
    explicit DynamicArray(uint64 initialCapacity)
    {
        Reserve(initialCapacity);
    }

    // Destructor
    ~DynamicArray()
    {
        Clear();
        if (m_data != nullptr)
        {
            free(m_data);
        }
    }

    // Copy constructor
    DynamicArray(const DynamicArray& other)
        : m_size(other.m_size)
        , m_capacity(other.m_capacity)
    {
        if (other.m_capacity > 0)
        {
            m_data = static_cast<T*>(malloc(m_capacity * sizeof(T)));
            for (uint64 i = 0; i < m_size; ++i)
            {
                new (&m_data[i]) T(other.m_data[i]);
            }
        }
    }

    // Copy assignment
    DynamicArray& operator=(const DynamicArray& other)
    {
        if (this != &other)
        {
            Clear();
            if (m_data != nullptr)
            {
                free(m_data);
            }

            m_size = other.m_size;
            m_capacity = other.m_capacity;

            if (other.m_capacity > 0)
            {
                m_data = static_cast<T*>(malloc(m_capacity * sizeof(T)));
                for (uint64 i = 0; i < m_size; ++i)
                {
                    new (&m_data[i]) T(other.m_data[i]);
                }
            }
            else
            {
                m_data = nullptr;
            }
        }
        return *this;
    }

    // Move constructor
    DynamicArray(DynamicArray&& other) noexcept
        : m_data(other.m_data)
        , m_size(other.m_size)
        , m_capacity(other.m_capacity)
    {
        other.m_data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
    }

    // Move assignment
    DynamicArray& operator=(DynamicArray&& other) noexcept
    {
        if (this != &other)
        {
            Clear();
            if (m_data != nullptr)
            {
                free(m_data);
            }

            m_data = other.m_data;
            m_size = other.m_size;
            m_capacity = other.m_capacity;

            other.m_data = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        }
        return *this;
    }

    // Add element at end
    void PushBack(const T& value)
    {
        if (m_size >= m_capacity)
        {
            Grow(m_size + 1);
        }
        new (&m_data[m_size]) T(value);
        ++m_size;
    }

    void PushBack(T&& value)
    {
        if (m_size >= m_capacity)
        {
            Grow(m_size + 1);
        }
        new (&m_data[m_size]) T(rsblMove(value));
        ++m_size;
    }

    // Remove last element
    void PopBack()
    {
        if (m_size > 0)
        {
            --m_size;
            m_data[m_size].~T();
        }
    }

    // Access elements
    T& operator[](uint64 index)
    {
        return m_data[index];
    }

    const T& operator[](uint64 index) const
    {
        return m_data[index];
    }

    // Get size
    uint64 Size() const
    {
        return m_size;
    }

    // Get capacity
    uint64 Capacity() const
    {
        return m_capacity;
    }

    // Check if empty
    bool IsEmpty() const
    {
        return m_size == 0;
    }

    // Reserve capacity
    void Reserve(uint64 newCapacity)
    {
        if (newCapacity > m_capacity)
        {
            Grow(newCapacity);
        }
    }

    // Resize array to new size
    // If newSize < current size, excess elements are destroyed
    // If newSize > current size, new elements are default-constructed
    void Resize(uint64 newSize)
    {
        if (newSize < m_size)
        {
            // Destroy excess elements
            for (uint64 i = newSize; i < m_size; ++i)
            {
                m_data[i].~T();
            }
            m_size = newSize;
        }
        else if (newSize > m_size)
        {
            // Grow capacity if needed
            if (newSize > m_capacity)
            {
                Grow(newSize);
            }

            // Default-construct new elements
            for (uint64 i = m_size; i < newSize; ++i)
            {
                new (&m_data[i]) T();
            }
            m_size = newSize;
        }
    }

    // Clear all elements
    void Clear()
    {
        for (uint64 i = 0; i < m_size; ++i)
        {
            m_data[i].~T();
        }
        m_size = 0;
    }

    // Get raw data pointer
    T* Data()
    {
        return m_data;
    }

    const T* Data() const
    {
        return m_data;
    }

    // Iterator support
    T* begin()
    {
        return m_data;
    }

    const T* begin() const
    {
        return m_data;
    }

    T* end()
    {
        return m_data + m_size;
    }

    const T* end() const
    {
        return m_data + m_size;
    }
};

} // namespace rsbl
