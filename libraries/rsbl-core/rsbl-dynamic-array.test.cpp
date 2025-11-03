// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "include/rsbl-core.h"
#include "include/rsbl-dynamic-array.h"

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

    TestStruct& operator=(const TestStruct&) = delete;

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

TEST_SUITE("rsbl::DynamicArray")
{
    TEST_CASE("Default constructor creates empty array")
    {
        DynamicArray<int> arr;

        CHECK(arr.Size() == 0);
        CHECK(arr.IsEmpty() == true);
        CHECK(arr.Capacity() == 0);
    }

    TEST_CASE("Constructor with initial capacity")
    {
        DynamicArray<int> arr(10);

        CHECK(arr.Size() == 0);
        CHECK(arr.Capacity() >= 10);
        CHECK(arr.IsEmpty() == true);
    }

    TEST_CASE("PushBack adds elements (lvalue)")
    {
        DynamicArray<int> arr;

        int val1 = 42;
        int val2 = 99;

        arr.PushBack(val1);
        arr.PushBack(val2);

        CHECK(arr.Size() == 2);
        CHECK(arr[0] == 42);
        CHECK(arr[1] == 99);
    }

    TEST_CASE("PushBack adds elements (rvalue)")
    {
        TestStruct::resetCounters();

        DynamicArray<TestStruct> arr;
        arr.PushBack(TestStruct(42));

        CHECK(arr.Size() == 1);
        CHECK(arr[0].value == 42);
        CHECK(TestStruct::moveConstructorCalls > 0);
    }

    TEST_CASE("PopBack removes last element")
    {
        DynamicArray<int> arr;
        arr.PushBack(1);
        arr.PushBack(2);
        arr.PushBack(3);

        CHECK(arr.Size() == 3);

        arr.PopBack();
        CHECK(arr.Size() == 2);
        CHECK(arr[1] == 2);

        arr.PopBack();
        CHECK(arr.Size() == 1);
        CHECK(arr[0] == 1);
    }

    TEST_CASE("PopBack on empty array does nothing")
    {
        DynamicArray<int> arr;
        arr.PopBack(); // Should not crash

        CHECK(arr.Size() == 0);
    }

    TEST_CASE("Operator[] accesses elements")
    {
        DynamicArray<int> arr;
        arr.PushBack(10);
        arr.PushBack(20);
        arr.PushBack(30);

        CHECK(arr[0] == 10);
        CHECK(arr[1] == 20);
        CHECK(arr[2] == 30);

        // Modify through operator[]
        arr[1] = 99;
        CHECK(arr[1] == 99);
    }

    TEST_CASE("Operator[] const accesses elements")
    {
        DynamicArray<int> arr;
        arr.PushBack(42);

        const DynamicArray<int>& constArr = arr;
        CHECK(constArr[0] == 42);
    }

    TEST_CASE("Clear removes all elements")
    {
        TestStruct::resetCounters();

        DynamicArray<TestStruct> arr;
        arr.PushBack(TestStruct(1));
        arr.PushBack(TestStruct(2));
        arr.PushBack(TestStruct(3));

        int destructorsBefore = TestStruct::destructorCalls;
        arr.Clear();

        CHECK(arr.Size() == 0);
        CHECK(arr.IsEmpty() == true);
        // Destructors should have been called for all elements
        CHECK(TestStruct::destructorCalls > destructorsBefore);
    }

    TEST_CASE("Reserve increases capacity")
    {
        DynamicArray<int> arr;
        CHECK(arr.Capacity() == 0);

        arr.Reserve(100);
        CHECK(arr.Capacity() >= 100);
        CHECK(arr.Size() == 0); // Size unchanged
    }

    TEST_CASE("Reserve does not decrease capacity")
    {
        DynamicArray<int> arr;
        arr.Reserve(100);
        uint64 capacity = arr.Capacity();

        arr.Reserve(50);
        CHECK(arr.Capacity() == capacity); // Unchanged
    }

    TEST_CASE("Array grows automatically when needed")
    {
        DynamicArray<int> arr;

        for (int i = 0; i < 100; ++i)
        {
            arr.PushBack(i);
        }

        CHECK(arr.Size() == 100);
        CHECK(arr.Capacity() >= 100);

        // Verify all elements
        for (int i = 0; i < 100; ++i)
        {
            CHECK(arr[static_cast<uint64>(i)] == i);
        }
    }

    TEST_CASE("Copy constructor creates independent copy")
    {
        DynamicArray<int> arr1;
        arr1.PushBack(1);
        arr1.PushBack(2);
        arr1.PushBack(3);

        DynamicArray<int> arr2(arr1);

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
        DynamicArray<int> arr1;
        arr1.PushBack(1);
        arr1.PushBack(2);

        DynamicArray<int> arr2;
        arr2.PushBack(99);

        arr2 = arr1;

        CHECK(arr2.Size() == 2);
        CHECK(arr2[0] == 1);
        CHECK(arr2[1] == 2);

        // Modify arr1, arr2 should be unchanged
        arr1[0] = 77;
        CHECK(arr2[0] == 1);
    }

    TEST_CASE("Copy assignment to self is safe")
    {
        DynamicArray<int> arr;
        arr.PushBack(42);

        arr = arr; // Self-assignment

        CHECK(arr.Size() == 1);
        CHECK(arr[0] == 42);
    }

    TEST_CASE("Move constructor transfers ownership")
    {
        DynamicArray<int> arr1;
        arr1.PushBack(1);
        arr1.PushBack(2);
        arr1.PushBack(3);

        DynamicArray<int> arr2(rsblMove(arr1));

        CHECK(arr2.Size() == 3);
        CHECK(arr2[0] == 1);
        CHECK(arr2[1] == 2);
        CHECK(arr2[2] == 3);

        // arr1 should be empty
        CHECK(arr1.Size() == 0);
        CHECK(arr1.Capacity() == 0);
    }

    TEST_CASE("Move assignment transfers ownership")
    {
        DynamicArray<int> arr1;
        arr1.PushBack(1);
        arr1.PushBack(2);

        DynamicArray<int> arr2;
        arr2 = rsblMove(arr1);

        CHECK(arr2.Size() == 2);
        CHECK(arr2[0] == 1);
        CHECK(arr2[1] == 2);

        // arr1 should be empty
        CHECK(arr1.Size() == 0);
        CHECK(arr1.Capacity() == 0);
    }

    TEST_CASE("Data() returns pointer to elements")
    {
        DynamicArray<int> arr;
        arr.PushBack(10);
        arr.PushBack(20);

        int* data = arr.Data();
        CHECK(data[0] == 10);
        CHECK(data[1] == 20);
    }

    TEST_CASE("Data() const returns const pointer")
    {
        DynamicArray<int> arr;
        arr.PushBack(42);

        const DynamicArray<int>& constArr = arr;
        const int* data = constArr.Data();
        CHECK(data[0] == 42);
    }

    TEST_CASE("Iterator support with range-based for")
    {
        DynamicArray<int> arr;
        arr.PushBack(1);
        arr.PushBack(2);
        arr.PushBack(3);

        int sum = 0;
        for (int val : arr)
        {
            sum += val;
        }

        CHECK(sum == 6);
    }

    TEST_CASE("Const iterator support")
    {
        DynamicArray<int> arr;
        arr.PushBack(10);
        arr.PushBack(20);

        const DynamicArray<int>& constArr = arr;

        int sum = 0;
        for (int val : constArr)
        {
            sum += val;
        }

        CHECK(sum == 30);
    }

    TEST_CASE("Destructor calls element destructors")
    {
        TestStruct::resetCounters();

        {
            DynamicArray<TestStruct> arr;
            arr.PushBack(TestStruct(1));
            arr.PushBack(TestStruct(2));
            arr.PushBack(TestStruct(3));

            CHECK(arr.Size() == 3);
        }

        // All elements should be destructed
        CHECK(TestStruct::destructorCalls >= 3);
    }

    TEST_CASE("Works with non-trivial types")
    {
        DynamicArray<TestStruct> arr;

        TestStruct::resetCounters();

        arr.PushBack(TestStruct(100));
        arr.PushBack(TestStruct(200));

        CHECK(arr.Size() == 2);
        CHECK(arr[0].value == 100);
        CHECK(arr[1].value == 200);
    }

    TEST_CASE("Growth strategy uses doubling")
    {
        DynamicArray<int> arr;

        // Push elements and track capacity growth
        arr.PushBack(1);
        uint64 prevCapacity = arr.Capacity();

        // Keep pushing until we trigger a grow
        for (int i = 2; i < 100; ++i)
        {
            arr.PushBack(i);
            if (arr.Capacity() > prevCapacity)
            {
                // Capacity should roughly double
                CHECK(arr.Capacity() >= prevCapacity * 2);
                prevCapacity = arr.Capacity();
            }
        }
    }
}
