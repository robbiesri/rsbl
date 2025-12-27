// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "include/rsbl-ptr.h"

using namespace rsbl;

// Test helper struct with tracking capabilities
struct TestStruct
{
    int value;
    static inline int constructorCalls = 0;
    static inline int destructorCalls = 0;

    TestStruct(int v = 0)
        : value(v)
    {
        constructorCalls++;
    }

    ~TestStruct()
    {
        destructorCalls++;
    }

    static void resetCounters()
    {
        constructorCalls = 0;
        destructorCalls = 0;
    }
};

TEST_SUITE("rsbl::UniquePtr")
{
    TEST_CASE("Default constructor creates null pointer")
    {
        UniquePtr<int> ptr;
        CHECK(ptr.Get() == nullptr);
        CHECK_FALSE(ptr);
    }

    TEST_CASE("Constructor from raw pointer takes ownership")
    {
        TestStruct::resetCounters();

        int* raw = new int(42);
        UniquePtr<int> ptr(raw);

        CHECK(ptr.Get() == raw);
        CHECK(*ptr == 42);
        CHECK(static_cast<bool>(ptr));
    }

    TEST_CASE("Destructor deletes managed object")
    {
        TestStruct::resetCounters();

        {
            UniquePtr<TestStruct> ptr(new TestStruct(42));
            CHECK(TestStruct::constructorCalls == 1);
            CHECK(TestStruct::destructorCalls == 0);
        }

        // After scope, destructor should have been called
        CHECK(TestStruct::destructorCalls == 1);
    }

    TEST_CASE("Move constructor transfers ownership")
    {
        TestStruct::resetCounters();

        UniquePtr<TestStruct> ptr1(new TestStruct(42));
        TestStruct* raw = ptr1.Get();

        UniquePtr<TestStruct> ptr2(rsblMove(ptr1));

        CHECK(ptr2.Get() == raw);
        CHECK(ptr2->value == 42);
        CHECK(ptr1.Get() == nullptr);
        CHECK_FALSE(ptr1);
        CHECK(TestStruct::destructorCalls == 0); // No deletion yet
    }

    TEST_CASE("Move assignment transfers ownership")
    {
        TestStruct::resetCounters();

        UniquePtr<TestStruct> ptr1(new TestStruct(42));
        UniquePtr<TestStruct> ptr2(new TestStruct(99));

        int destructorsBefore = TestStruct::destructorCalls;
        ptr2 = rsblMove(ptr1);

        CHECK(ptr2->value == 42);
        CHECK(ptr1.Get() == nullptr);
        // Original ptr2 object should have been deleted
        CHECK(TestStruct::destructorCalls == destructorsBefore + 1);
    }

    TEST_CASE("Move assignment to itself is safe")
    {
        TestStruct::resetCounters();

        UniquePtr<TestStruct> ptr(new TestStruct(42));
        auto& ref = ptr;

        ptr = rsblMove(ref);

        // Self-move should be safe (though object may be in valid but unspecified state)
        // At minimum, it shouldn't crash
    }

    TEST_CASE("Dereference operator returns reference")
    {
        UniquePtr<int> ptr(new int(42));

        *ptr = 100;
        CHECK(*ptr == 100);
    }

    TEST_CASE("Arrow operator provides access to members")
    {
        UniquePtr<TestStruct> ptr(new TestStruct(42));

        CHECK(ptr->value == 42);
        ptr->value = 100;
        CHECK(ptr->value == 100);
    }

    TEST_CASE("Get returns raw pointer")
    {
        int* raw = new int(42);
        UniquePtr<int> ptr(raw);

        CHECK(ptr.Get() == raw);
    }

    TEST_CASE("Release transfers ownership without deleting")
    {
        TestStruct::resetCounters();

        UniquePtr<TestStruct> ptr(new TestStruct(42));
        TestStruct* raw = ptr.Release();

        CHECK(ptr.Get() == nullptr);
        CHECK(raw->value == 42);
        CHECK(TestStruct::destructorCalls == 0); // Not deleted

        // Manually clean up
        delete raw;
        CHECK(TestStruct::destructorCalls == 1);
    }

    TEST_CASE("Reset with nullptr deletes current object")
    {
        TestStruct::resetCounters();

        UniquePtr<TestStruct> ptr(new TestStruct(42));
        CHECK(TestStruct::destructorCalls == 0);

        ptr.Reset();

        CHECK(ptr.Get() == nullptr);
        CHECK(TestStruct::destructorCalls == 1);
    }

    TEST_CASE("Reset with new pointer deletes old and takes new")
    {
        TestStruct::resetCounters();

        UniquePtr<TestStruct> ptr(new TestStruct(42));
        TestStruct* newObj = new TestStruct(99);

        ptr.Reset(newObj);

        CHECK(ptr.Get() == newObj);
        CHECK(ptr->value == 99);
        CHECK(TestStruct::destructorCalls == 1); // Old object deleted
    }

    TEST_CASE("Bool operator returns false for null pointer")
    {
        UniquePtr<int> ptr;
        CHECK_FALSE(ptr);
    }

    TEST_CASE("Bool operator returns true for valid pointer")
    {
        UniquePtr<int> ptr(new int(42));
        CHECK(static_cast<bool>(ptr));
    }

    TEST_CASE("MakeUnique creates UniquePtr with constructed object")
    {
        TestStruct::resetCounters();

        auto ptr = MakeUnique<TestStruct>(42);

        CHECK(ptr->value == 42);
        CHECK(TestStruct::constructorCalls == 1);
    }

    TEST_CASE("MakeUnique with no arguments")
    {
        TestStruct::resetCounters();

        auto ptr = MakeUnique<TestStruct>();

        CHECK(ptr->value == 0);
        CHECK(TestStruct::constructorCalls == 1);
    }

    TEST_CASE("Multiple UniquePtrs with same type")
    {
        TestStruct::resetCounters();

        {
            UniquePtr<TestStruct> ptr1(new TestStruct(1));
            UniquePtr<TestStruct> ptr2(new TestStruct(2));
            UniquePtr<TestStruct> ptr3(new TestStruct(3));

            CHECK(ptr1->value == 1);
            CHECK(ptr2->value == 2);
            CHECK(ptr3->value == 3);
        }

        // All three should be deleted
        CHECK(TestStruct::destructorCalls == 3);
    }

    TEST_CASE("Null pointer Reset is safe")
    {
        UniquePtr<TestStruct> ptr;
        ptr.Reset(); // Should not crash

        CHECK(ptr.Get() == nullptr);
    }

    TEST_CASE("Move from null pointer")
    {
        UniquePtr<int> ptr1;
        UniquePtr<int> ptr2(rsblMove(ptr1));

        CHECK(ptr1.Get() == nullptr);
        CHECK(ptr2.Get() == nullptr);
    }
}
