// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "include/rsbl-core.h"
#include "include/rsbl-result.h"

using namespace rsbl;

// Test helper struct with tracking capabilities
struct TestStruct
{
    int value;
    static inline int constructorCalls = 0;
    static inline int destructorCalls = 0;
    static inline int moveConstructorCalls = 0;

    TestStruct(int v = 0)
        : value(v)
    {
        constructorCalls++;
    }

    TestStruct(const TestStruct& other)
        : value(other.value)
    {
        constructorCalls++;
    }

    TestStruct(TestStruct&& other) noexcept
        : value(other.value)
    {
        moveConstructorCalls++;
        other.value = -1; // Mark as moved
    }

    // MSVC C4626 wants this explicitly deleted vs implicit...
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
    }
};

TEST_SUITE("rsbl::Result")
{
    TEST_CASE("Construction with value (lvalue)")
    {
        TestStruct::resetCounters();

        TestStruct val(42);
        Result<TestStruct> result(val);

        CHECK(result.Code() == ResultCode::Success);
        CHECK(result.Value().value == 42);
        CHECK(TestStruct::constructorCalls == 2); // original + copy
    }

    TEST_CASE("Construction with value (rvalue)")
    {
        TestStruct::resetCounters();

        Result<TestStruct> result(TestStruct(42));

        CHECK(result.Code() == ResultCode::Success);
        CHECK(result.Value().value == 42);
        CHECK(TestStruct::moveConstructorCalls == 1);
    }

    TEST_CASE("Construction with ResultCode::Success")
    {
        TestStruct::resetCounters();

        Result<TestStruct> result(ResultCode::Success);

        CHECK(result.Code() == ResultCode::Success);
        CHECK(result.Value().value == 0); // Default constructed
        CHECK(TestStruct::constructorCalls == 1);
    }

    TEST_CASE("Construction with ResultCode::Failure")
    {
        TestStruct::resetCounters();

        Result<TestStruct> result(ResultCode::Failure);

        CHECK(result.Code() == ResultCode::Failure);
        // No ReturnType should be constructed for failure
        CHECK(TestStruct::constructorCalls == 0);
    }

    TEST_CASE("Construction with failure text")
    {
        Result<int> result("Test error message");

        CHECK(result.Code() == ResultCode::Failure);
        CHECK(std::string(result.FailureText()) == "Test error message");
    }

    TEST_CASE("Move constructor")
    {
        TestStruct::resetCounters();

        Result<TestStruct> original(TestStruct(42));
        Result<TestStruct> moved(rsblMove(original));

        CHECK(moved.Code() == ResultCode::Success);
        CHECK(moved.Value().value == 42);
    }

    TEST_CASE("Move assignment operator")
    {
        TestStruct::resetCounters();

        Result<TestStruct> result1(TestStruct(42));
        Result<TestStruct> result2(TestStruct(99));

        result2 = rsblMove(result1);

        CHECK(result2.Code() == ResultCode::Success);
        CHECK(result2.Value().value == 42);
        CHECK(result1.Code() == ResultCode::Failure); // moved-from is invalidated
    }

    TEST_CASE("Move assignment from failure to success calls destructor")
    {
        TestStruct::resetCounters();

        Result<TestStruct> success(TestStruct(42));
        Result<TestStruct> failure(ResultCode::Failure);

        int destructorsBeforeMove = TestStruct::destructorCalls;
        success = rsblMove(failure);

        CHECK(success.Code() == ResultCode::Failure);
        // Original value in 'success' should have been destructed
        CHECK(TestStruct::destructorCalls == destructorsBeforeMove + 1);
    }

    TEST_CASE("Value() returns reference to stored value")
    {
        Result<TestStruct> result(TestStruct(42));

        // Modify through reference
        result.Value().value = 100;

        CHECK(result.Value().value == 100);
    }

    TEST_CASE("Value() const returns const reference")
    {
        const Result<TestStruct> result(TestStruct(42));

        CHECK(result.Value().value == 42);
        // This should compile as const reference
        const TestStruct& val = result.Value();
        CHECK(val.value == 42);
    }

    TEST_CASE("Destructor calls ReturnType destructor on success")
    {
        TestStruct::resetCounters();

        {
            Result<TestStruct> result(TestStruct(42));
            CHECK(TestStruct::destructorCalls == 1); // Initial instance was copied into Result
        }

        // After scope, destructor should have been called
        CHECK(TestStruct::destructorCalls > 1);
    }

    TEST_CASE("Destructor does not call ReturnType destructor on failure")
    {
        TestStruct::resetCounters();

        {
            Result<TestStruct> result(ResultCode::Failure);
        }

        // No TestStruct was constructed, so no destructor calls
        CHECK(TestStruct::destructorCalls == 0);
    }

    TEST_CASE("Default return type (empty Result)")
    {
        Result<> emptySuccess(ResultCode::Success);
        Result<> emptyFailure(ResultCode::Failure);

        CHECK(emptySuccess.Code() == ResultCode::Success);
        CHECK(emptyFailure.Code() == ResultCode::Failure);
    }

    TEST_CASE("Multiple failures update failure text")
    {
        Result<int> result1("First error");
        CHECK(std::string(result1.FailureText()) == "First error");

        Result<int> result2("Second error");
        // Failure text is thread-local, so it should be updated
        CHECK(std::string(result2.FailureText()) == "Second error");
    }

    TEST_CASE("FailureText persists across Result instances")
    {
        {
            Result<int> result1("Persistent error");
        }

        // Create new Result and check if error text persists (it's thread-local)
        Result<int> result2(ResultCode::Success);
        CHECK(std::string(result2.FailureText()) == "Persistent error");
    }

    TEST_CASE("Integer return type")
    {
        Result<int> result(42);

        CHECK(result.Code() == ResultCode::Success);
        CHECK(result.Value() == 42);
    }

    TEST_CASE("Pointer return type")
    {
        int x = 42;
        Result<int*> result(&x);

        CHECK(result.Code() == ResultCode::Success);
        CHECK(*result.Value() == 42);
    }

    TEST_CASE("Move assignment to itself (edge case)")
    {
        TestStruct::resetCounters();

        Result<TestStruct> result(TestStruct(42));

        // Self-assignment (though unusual, should not crash)
        // Note: The TODO in the implementation mentions not checking for self-assignment
        // This test documents the current behavior
        auto& ref = result;
        result = rsblMove(ref);

        // After self-move, state is implementation-defined but should not crash
        CHECK(result.Code() == ResultCode::Failure); // Gets invalidated
    }

    TEST_CASE("ResultCode enum values")
    {
        CHECK(static_cast<uint8>(ResultCode::Success) == 0);
        CHECK(static_cast<uint8>(ResultCode::Failure) == 1);
    }

    TEST_CASE("Move construction from failure")
    {
        Result<TestStruct> original(ResultCode::Failure);
        Result<TestStruct> moved(rsblMove(original));

        CHECK(moved.Code() == ResultCode::Failure);
        CHECK(original.Code() == ResultCode::Failure);
    }

    TEST_CASE("Chained move assignments")
    {
        Result<int> r1(42);
        Result<int> r2(99);
        Result<int> r3(100);

        r3 = rsblMove(r2);
        r2 = rsblMove(r1);

        CHECK(r3.Value() == 99);
        CHECK(r2.Value() == 42);
        CHECK(r1.Code() == ResultCode::Failure);
    }
}
