// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "include/rsbl-core.h"
#include "include/rsbl-fixed-array.h"

using namespace rsbl;

// Test helper struct with tracking capabilities
struct TestStruct
{
    int value;
    static inline int constructorCalls = 0;
    static inline int destructorCalls = 0;
    static inline int moveConstructorCalls = 0;
    static inline int copyConstructorCalls = 0;

    TestStruct(int v = 0)
        : value(v)
    {
        constructorCalls++;
    }

    TestStruct(const TestStruct& other)
        : value(other.value)
    {
        copyConstructorCalls++;
    }

    TestStruct(TestStruct&& other) noexcept
        : value(other.value)
    {
        moveConstructorCalls++;
        other.value = -1; // Mark as moved
    }

    TestStruct& operator=(const TestStruct& other)
    {
        value = other.value;
        return *this;
    }

    TestStruct& operator=(TestStruct&& other) noexcept
    {
        value = other.value;
        other.value = -1;
        return *this;
    }

    ~TestStruct()
    {
        destructorCalls++;
    }

    static void resetCounters()
    {
        constructorCalls = 0;
        destructorCalls = 0;
        moveConstructorCalls = 0;
        copyConstructorCalls = 0;
    }
};

TEST_SUITE("rsbl::FixedArray")
{
    TEST_CASE("Default constructor creates array with default values")
    {
        FixedArray<int, 5> arr;

        CHECK(arr.Size() == 5);
    }

    TEST_CASE("Size is compile-time constant")
    {
        FixedArray<int, 10> arr;

        constexpr uint64 size = arr.Size();
        CHECK(size == 10);
    }

    TEST_CASE("CTAD deduces type and size from initializer list")
    {
        FixedArray arr = {1, 2, 3, 4, 5};

        CHECK(arr.Size() == 5);
        CHECK(arr[0] == 1);
        CHECK(arr[1] == 2);
        CHECK(arr[2] == 3);
        CHECK(arr[3] == 4);
        CHECK(arr[4] == 5);
    }

    TEST_CASE("CTAD works with different types")
    {
        FixedArray intArr = {1, 2, 3};
        FixedArray floatArr = {1.0f, 2.0f, 3.0f};

        CHECK(intArr.Size() == 3);
        CHECK(floatArr.Size() == 3);
    }

    TEST_CASE("Operator[] accesses elements")
    {
        FixedArray<int, 3> arr;
        arr[0] = 10;
        arr[1] = 20;
        arr[2] = 30;

        CHECK(arr[0] == 10);
        CHECK(arr[1] == 20);
        CHECK(arr[2] == 30);

        // Modify through operator[]
        arr[1] = 99;
        CHECK(arr[1] == 99);
    }

    TEST_CASE("Operator[] const accesses elements")
    {
        FixedArray<int, 2> arr;
        arr[0] = 42;
        arr[1] = 99;

        const FixedArray<int, 2>& constArr = arr;
        CHECK(constArr[0] == 42);
        CHECK(constArr[1] == 99);
    }

    TEST_CASE("Fill sets all elements to value")
    {
        FixedArray<int, 5> arr;
        arr.Fill(42);

        for (uint64 i = 0; i < arr.Size(); ++i)
        {
            CHECK(arr[i] == 42);
        }
    }

    TEST_CASE("Copy constructor creates independent copy")
    {
        FixedArray<int, 3> arr1;
        arr1[0] = 1;
        arr1[1] = 2;
        arr1[2] = 3;

        FixedArray<int, 3> arr2(arr1);

        CHECK(arr2.Size() == 3);
        CHECK(arr2[0] == 1);
        CHECK(arr2[1] == 2);
        CHECK(arr2[2] == 3);

        // Modify arr1, arr2 should be unchanged
        arr1[0] = 99;
        CHECK(arr2[0] == 1);
    }

    TEST_CASE("Copy assignment creates independent copy")
    {
        FixedArray<int, 2> arr1;
        arr1[0] = 1;
        arr1[1] = 2;

        FixedArray<int, 2> arr2;
        arr2[0] = 99;
        arr2[1] = 88;

        arr2 = arr1;

        CHECK(arr2[0] == 1);
        CHECK(arr2[1] == 2);

        // Modify arr1, arr2 should be unchanged
        arr1[0] = 77;
        CHECK(arr2[0] == 1);
    }

    TEST_CASE("Copy assignment to self is safe")
    {
        FixedArray<int, 2> arr;
        arr[0] = 42;
        arr[1] = 99;

        arr = arr; // Self-assignment

        CHECK(arr.Size() == 2);
        CHECK(arr[0] == 42);
        CHECK(arr[1] == 99);
    }

    TEST_CASE("Move constructor transfers values")
    {
        FixedArray<int, 3> arr1;
        arr1[0] = 1;
        arr1[1] = 2;
        arr1[2] = 3;

        FixedArray<int, 3> arr2(rsblMove(arr1));

        CHECK(arr2.Size() == 3);
        CHECK(arr2[0] == 1);
        CHECK(arr2[1] == 2);
        CHECK(arr2[2] == 3);
    }

    TEST_CASE("Move assignment transfers values")
    {
        FixedArray<int, 2> arr1;
        arr1[0] = 1;
        arr1[1] = 2;

        FixedArray<int, 2> arr2;
        arr2 = rsblMove(arr1);

        CHECK(arr2[0] == 1);
        CHECK(arr2[1] == 2);
    }

    TEST_CASE("Data() returns pointer to elements")
    {
        FixedArray<int, 2> arr;
        arr[0] = 10;
        arr[1] = 20;

        int* data = arr.Data();
        CHECK(data[0] == 10);
        CHECK(data[1] == 20);

        // Modify through pointer
        data[0] = 99;
        CHECK(arr[0] == 99);
    }

    TEST_CASE("Data() const returns const pointer")
    {
        FixedArray<int, 2> arr;
        arr[0] = 42;
        arr[1] = 99;

        const FixedArray<int, 2>& constArr = arr;
        const int* data = constArr.Data();
        CHECK(data[0] == 42);
        CHECK(data[1] == 99);
    }

    TEST_CASE("Iterator support with range-based for")
    {
        FixedArray<int, 3> arr;
        arr[0] = 1;
        arr[1] = 2;
        arr[2] = 3;

        int sum = 0;
        for (int val : arr)
        {
            sum += val;
        }

        CHECK(sum == 6);
    }

    TEST_CASE("Const iterator support")
    {
        FixedArray<int, 2> arr;
        arr[0] = 10;
        arr[1] = 20;

        const FixedArray<int, 2>& constArr = arr;

        int sum = 0;
        for (int val : constArr)
        {
            sum += val;
        }

        CHECK(sum == 30);
    }

    TEST_CASE("Modifying elements through range-based for")
    {
        FixedArray<int, 3> arr;
        arr[0] = 1;
        arr[1] = 2;
        arr[2] = 3;

        for (int& val : arr)
        {
            val *= 2;
        }

        CHECK(arr[0] == 2);
        CHECK(arr[1] == 4);
        CHECK(arr[2] == 6);
    }

    TEST_CASE("Works with non-trivial types")
    {
        TestStruct::resetCounters();

        FixedArray<TestStruct, 2> arr;
        arr[0] = TestStruct(100);
        arr[1] = TestStruct(200);

        CHECK(arr[0].value == 100);
        CHECK(arr[1].value == 200);
    }



    TEST_CASE("Works with large arrays")
    {
        FixedArray<int, 1000> arr;
        arr.Fill(42);

        CHECK(arr.Size() == 1000);
        CHECK(arr[0] == 42);
        CHECK(arr[500] == 42);
        CHECK(arr[999] == 42);
    }

    TEST_CASE("Multiple arrays with different sizes")
    {
        FixedArray<int, 3> small;
        FixedArray<int, 100> large;

        small.Fill(1);
        large.Fill(2);

        CHECK(small.Size() == 3);
        CHECK(large.Size() == 100);
        CHECK(small[0] == 1);
        CHECK(large[0] == 2);
    }
}
