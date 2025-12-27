// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "include/rsbl-assert.h"
#include "include/rsbl-math-types.h"

using namespace rsbl;

TEST_SUITE("rsbl::uint2")
{
    TEST_CASE("Default constructor")
    {
        uint2 v;
        // Note: Default constructor leaves values uninitialized
        // This just tests that it compiles
        rsblUnused(v);
    }

    TEST_CASE("Single-value constructor")
    {
        uint2 v(42);
        CHECK(v.x == 42);
        CHECK(v.y == 42);
    }

    TEST_CASE("Component constructor")
    {
        uint2 v(10, 20);
        CHECK(v.x == 10);
        CHECK(v.y == 20);
    }

    TEST_CASE("Zero initialization")
    {
        uint2 v(0);
        CHECK(v.x == 0);
        CHECK(v.y == 0);
    }
}

TEST_SUITE("rsbl::uint3")
{
    TEST_CASE("Default constructor")
    {
        uint3 v;
        // Note: Default constructor leaves values uninitialized
        // This just tests that it compiles
        rsblUnused(v);
    }

    TEST_CASE("Single-value constructor")
    {
        uint3 v(42);
        CHECK(v.x == 42);
        CHECK(v.y == 42);
        CHECK(v.z == 42);
    }

    TEST_CASE("Component constructor")
    {
        uint3 v(10, 20, 30);
        CHECK(v.x == 10);
        CHECK(v.y == 20);
        CHECK(v.z == 30);
    }

    TEST_CASE("Zero initialization")
    {
        uint3 v(0);
        CHECK(v.x == 0);
        CHECK(v.y == 0);
        CHECK(v.z == 0);
    }
}

TEST_SUITE("rsbl::uint4")
{
    TEST_CASE("Default constructor")
    {
        uint4 v;
        // Note: Default constructor leaves values uninitialized
        // This just tests that it compiles
        rsblUnused(v);
    }

    TEST_CASE("Single-value constructor")
    {
        uint4 v(42);
        CHECK(v.x == 42);
        CHECK(v.y == 42);
        CHECK(v.z == 42);
        CHECK(v.w == 42);
    }

    TEST_CASE("Component constructor")
    {
        uint4 v(10, 20, 30, 40);
        CHECK(v.x == 10);
        CHECK(v.y == 20);
        CHECK(v.z == 30);
        CHECK(v.w == 40);
    }

    TEST_CASE("Zero initialization")
    {
        uint4 v(0);
        CHECK(v.x == 0);
        CHECK(v.y == 0);
        CHECK(v.z == 0);
        CHECK(v.w == 0);
    }
}

TEST_SUITE("rsbl::int2")
{
    TEST_CASE("Default constructor")
    {
        int2 v;
        // Note: Default constructor leaves values uninitialized
        // This just tests that it compiles
        rsblUnused(v);
    }

    TEST_CASE("Single-value constructor")
    {
        int2 v(42);
        CHECK(v.x == 42);
        CHECK(v.y == 42);
    }

    TEST_CASE("Component constructor")
    {
        int2 v(10, 20);
        CHECK(v.x == 10);
        CHECK(v.y == 20);
    }

    TEST_CASE("Negative values")
    {
        int2 v(-10, -20);
        CHECK(v.x == -10);
        CHECK(v.y == -20);
    }

    TEST_CASE("Zero initialization")
    {
        int2 v(0);
        CHECK(v.x == 0);
        CHECK(v.y == 0);
    }
}

TEST_SUITE("rsbl::int3")
{
    TEST_CASE("Default constructor")
    {
        int3 v;
        // Note: Default constructor leaves values uninitialized
        // This just tests that it compiles
        rsblUnused(v);
    }

    TEST_CASE("Single-value constructor")
    {
        int3 v(42);
        CHECK(v.x == 42);
        CHECK(v.y == 42);
        CHECK(v.z == 42);
    }

    TEST_CASE("Component constructor")
    {
        int3 v(10, 20, 30);
        CHECK(v.x == 10);
        CHECK(v.y == 20);
        CHECK(v.z == 30);
    }

    TEST_CASE("Negative values")
    {
        int3 v(-10, -20, -30);
        CHECK(v.x == -10);
        CHECK(v.y == -20);
        CHECK(v.z == -30);
    }

    TEST_CASE("Zero initialization")
    {
        int3 v(0);
        CHECK(v.x == 0);
        CHECK(v.y == 0);
        CHECK(v.z == 0);
    }
}

TEST_SUITE("rsbl::int4")
{
    TEST_CASE("Default constructor")
    {
        int4 v;
        // Note: Default constructor leaves values uninitialized
        // This just tests that it compiles
        rsblUnused(v);
    }

    TEST_CASE("Single-value constructor")
    {
        int4 v(42);
        CHECK(v.x == 42);
        CHECK(v.y == 42);
        CHECK(v.z == 42);
        CHECK(v.w == 42);
    }

    TEST_CASE("Component constructor")
    {
        int4 v(10, 20, 30, 40);
        CHECK(v.x == 10);
        CHECK(v.y == 20);
        CHECK(v.z == 30);
        CHECK(v.w == 40);
    }

    TEST_CASE("Negative values")
    {
        int4 v(-10, -20, -30, -40);
        CHECK(v.x == -10);
        CHECK(v.y == -20);
        CHECK(v.z == -30);
        CHECK(v.w == -40);
    }

    TEST_CASE("Mixed positive and negative")
    {
        int4 v(-10, 20, -30, 40);
        CHECK(v.x == -10);
        CHECK(v.y == 20);
        CHECK(v.z == -30);
        CHECK(v.w == 40);
    }

    TEST_CASE("Zero initialization")
    {
        int4 v(0);
        CHECK(v.x == 0);
        CHECK(v.y == 0);
        CHECK(v.z == 0);
        CHECK(v.w == 0);
    }
}
