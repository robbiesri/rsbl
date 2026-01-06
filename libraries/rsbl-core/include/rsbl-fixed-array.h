// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#pragma once

#include "rsbl-int-types.h"

namespace rsbl
{

// Fixed-size array similar to std::array
// Size is known at compile time, no dynamic allocation
// This is an aggregate type, so copy/move are compiler-generated

template <typename T, uint64 N>
struct FixedArray
{
    T m_data[N];

    // Access elements
    constexpr T& operator[](uint64 index)
    {
        return m_data[index];
    }

    constexpr const T& operator[](uint64 index) const
    {
        return m_data[index];
    }

    // Get size (compile-time constant)
    constexpr uint64 Size() const
    {
        return N;
    }

    // Fill all elements with value
    constexpr void Fill(const T& value)
    {
        for (uint64 i = 0; i < N; ++i)
        {
            m_data[i] = value;
        }
    }

    // Get raw data pointer
    constexpr T* Data()
    {
        return m_data;
    }

    constexpr const T* Data() const
    {
        return m_data;
    }

    // Iterator support
    constexpr T* begin()
    {
        return m_data;
    }

    constexpr const T* begin() const
    {
        return m_data;
    }

    constexpr T* end()
    {
        return m_data + N;
    }

    constexpr const T* end() const
    {
        return m_data + N;
    }
};

// Deduction guide for CTAD (Class Template Argument Deduction)
// Allows: FixedArray arr = {1, 2, 3}; to deduce FixedArray<int, 3>
template <typename T, typename... U>
FixedArray(T, U...) -> FixedArray<T, 1 + sizeof...(U)>;

} // namespace rsbl
