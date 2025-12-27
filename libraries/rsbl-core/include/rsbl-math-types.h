// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#pragma once

#include "rsbl-int-types.h"

// TODO: constructors that take in fixed size arrays
// TODO: interop with DirectXMath types

namespace rsbl
{

// 2-component unsigned integer vector
struct uint2
{
    uint32 x;
    uint32 y;

    uint2() = default;
    explicit uint2(uint32 v)
        : x(v)
        , y(v)
    {
    }
    uint2(uint32 x_, uint32 y_)
        : x(x_)
        , y(y_)
    {
    }
};

// 3-component unsigned integer vector
struct uint3
{
    uint32 x;
    uint32 y;
    uint32 z;

    uint3() = default;
    explicit uint3(uint32 v)
        : x(v)
        , y(v)
        , z(v)
    {
    }
    uint3(uint32 x_, uint32 y_, uint32 z_)
        : x(x_)
        , y(y_)
        , z(z_)
    {
    }
};

// 4-component unsigned integer vector
struct uint4
{
    uint32 x;
    uint32 y;
    uint32 z;
    uint32 w;

    uint4() = default;
    explicit uint4(uint32 v)
        : x(v)
        , y(v)
        , z(v)
        , w(v)
    {
    }
    uint4(uint32 x_, uint32 y_, uint32 z_, uint32 w_)
        : x(x_)
        , y(y_)
        , z(z_)
        , w(w_)
    {
    }
};

// 2-component signed integer vector
struct int2
{
    int32 x;
    int32 y;

    int2() = default;
    explicit int2(int32 v)
        : x(v)
        , y(v)
    {
    }
    int2(int32 x_, int32 y_)
        : x(x_)
        , y(y_)
    {
    }
};

// 3-component signed integer vector
struct int3
{
    int32 x;
    int32 y;
    int32 z;

    int3() = default;
    explicit int3(int32 v)
        : x(v)
        , y(v)
        , z(v)
    {
    }
    int3(int32 x_, int32 y_, int32 z_)
        : x(x_)
        , y(y_)
        , z(z_)
    {
    }
};

// 4-component signed integer vector
struct int4
{
    int32 x;
    int32 y;
    int32 z;
    int32 w;

    int4() = default;
    explicit int4(int32 v)
        : x(v)
        , y(v)
        , z(v)
        , w(v)
    {
    }
    int4(int32 x_, int32 y_, int32 z_, int32 w_)
        : x(x_)
        , y(y_)
        , z(z_)
        , w(w_)
    {
    }
};

} // namespace rsbl
