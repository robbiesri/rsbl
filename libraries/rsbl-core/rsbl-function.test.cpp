// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "include/rsbl-function.h"

using namespace rsbl;

// Test helper struct with tracking capabilities
struct FunctorTracker
{
    int value;
    static inline int constructorCalls = 0;
    static inline int destructorCalls = 0;
    static inline int moveConstructorCalls = 0;
    static inline int callOperatorCalls = 0;

    FunctorTracker(int v = 0)
        : value(v)
    {
        constructorCalls++;
    }

    FunctorTracker(const FunctorTracker& other)
        : value(other.value)
    {
        constructorCalls++;
    }

    FunctorTracker(FunctorTracker&& other) noexcept
        : value(other.value)
    {
        moveConstructorCalls++;
        other.value = -1;
    }

    ~FunctorTracker()
    {
        destructorCalls++;
    }

    int operator()(int x) const
    {
        callOperatorCalls++;
        return value + x;
    }

    static void resetCounters()
    {
        constructorCalls = 0;
        destructorCalls = 0;
        moveConstructorCalls = 0;
        callOperatorCalls = 0;
    }
};

// Test class for member function binding
struct Calculator
{
    int base = 10;

    int Add(int x)
    {
        return base + x;
    }

    int Multiply(int x) const
    {
        return base * x;
    }

    int AddTwo(int x, int y)
    {
        return base + x + y;
    }
};

// Free function for testing
int FreeAdd(int a, int b)
{
    return a + b;
}

TEST_SUITE("rsbl::Function")
{
    TEST_CASE("Default construction creates empty function")
    {
        Function<int(int)> func;

        CHECK_FALSE(func.Valid());
        CHECK_FALSE(func);
    }

    TEST_CASE("Construction with lambda")
    {
        auto lambda = [](int x) { return x * 2; };
        Function<int(int)> func(lambda);

        CHECK(func.Valid());
        CHECK(func);
        CHECK(func(5) == 10);
        CHECK(func(100) == 200);
    }

    TEST_CASE("Construction with capturing lambda")
    {
        int multiplier = 3;
        auto lambda = [multiplier](int x) { return x * multiplier; };
        Function<int(int)> func(lambda);

        CHECK(func.Valid());
        CHECK(func(5) == 15);
        CHECK(func(10) == 30);
    }

    TEST_CASE("Construction with mutable lambda")
    {
        int counter = 0;
        auto lambda = [counter](int x) mutable {
            counter++;
            return x + counter;
        };
        Function<int(int)> func(rsblMove(lambda));

        CHECK(func(10) == 11);
        CHECK(func(10) == 12);
        CHECK(func(10) == 13);
    }

    TEST_CASE("Invoke with different argument types (type conversion)")
    {
        auto lambda = [](int x) { return x * 2; };
        Function<int(int)> func(lambda);

        // Should work with implicit conversion from short to int
        short s = 5;
        CHECK(func(s) == 10);

        // Should work with implicit conversion from long to int
        long l = 10;
        CHECK(func(l) == 20);
    }

    TEST_CASE("Function returning void")
    {
        int called = 0;
        auto lambda = [&called]() { called++; };
        Function<void()> func(lambda);

        CHECK(func.Valid());
        func();
        CHECK(called == 1);
        func();
        CHECK(called == 2);
    }

    TEST_CASE("Function with no arguments")
    {
        auto lambda = []() { return 42; };
        Function<int()> func(lambda);

        CHECK(func() == 42);
    }

    TEST_CASE("Function with multiple arguments")
    {
        auto lambda = [](int a, int b, int c) { return a + b + c; };
        Function<int(int, int, int)> func(lambda);

        CHECK(func(1, 2, 3) == 6);
        CHECK(func(10, 20, 30) == 60);
    }

    TEST_CASE("Move constructor")
    {
        auto lambda = [](int x) { return x * 2; };
        Function<int(int)> func1(lambda);
        Function<int(int)> func2(rsblMove(func1));

        CHECK_FALSE(func1.Valid());
        CHECK(func2.Valid());
        CHECK(func2(5) == 10);
    }

    TEST_CASE("Move assignment")
    {
        auto lambda1 = [](int x) { return x * 2; };
        auto lambda2 = [](int x) { return x + 100; };

        Function<int(int)> func1(lambda1);
        Function<int(int)> func2(lambda2);

        CHECK(func1(5) == 10);
        CHECK(func2(5) == 105);

        func2 = rsblMove(func1);

        CHECK_FALSE(func1.Valid());
        CHECK(func2.Valid());
        CHECK(func2(5) == 10);
    }

    TEST_CASE("Move assignment from empty function")
    {
        auto lambda = [](int x) { return x * 2; };
        Function<int(int)> func1;
        Function<int(int)> func2(lambda);

        CHECK(func2(5) == 10);

        func2 = rsblMove(func1);

        CHECK_FALSE(func1.Valid());
        CHECK_FALSE(func2.Valid());
    }

    TEST_CASE("Move assignment to empty function")
    {
        auto lambda = [](int x) { return x * 2; };
        Function<int(int)> func1(lambda);
        Function<int(int)> func2;

        func2 = rsblMove(func1);

        CHECK_FALSE(func1.Valid());
        CHECK(func2.Valid());
        CHECK(func2(5) == 10);
    }

    TEST_CASE("Self-move assignment")
    {
        auto lambda = [](int x) { return x * 2; };
        Function<int(int)> func(lambda);

        auto& ref = func;
        func = rsblMove(ref);

        // After self-move, function should be reset
        CHECK_FALSE(func.Valid());
    }

    TEST_CASE("Destructor calls functor destructor")
    {
        FunctorTracker::resetCounters();

        {
            FunctorTracker tracker(10);
            Function<int(int)> func(rsblMove(tracker));

            CHECK(func(5) == 15);
            CHECK(FunctorTracker::callOperatorCalls == 1);
        }

        // Destructor should have been called for the functor inside Function
        CHECK(FunctorTracker::destructorCalls > 0);
    }

    TEST_CASE("Move from function calls destructor on moved-from functor")
    {
        FunctorTracker::resetCounters();

        Function<int(int)> func1;
        {
            FunctorTracker tracker(10);
            func1 = Function<int(int)>(rsblMove(tracker));
        }

        int destructorsBeforeMove = FunctorTracker::destructorCalls;

        Function<int(int)> func2(rsblMove(func1));

        CHECK(func2(5) == 15);
        // Moving should call destructor on the functor in func1
        CHECK(FunctorTracker::destructorCalls == destructorsBeforeMove + 1);
    }

    TEST_CASE("BindMember with non-const member function")
    {
        Calculator calc;
        calc.base = 10;

        auto bound = BindMember<&Calculator::Add>(&calc);
        Function<int(int)> func(bound);

        CHECK(func(5) == 15);
        CHECK(func(10) == 20);

        calc.base = 20;
        CHECK(func(5) == 25);
    }

    TEST_CASE("BindMember with const member function")
    {
        Calculator calc;
        calc.base = 3;

        auto bound = BindMember<&Calculator::Multiply>(&calc);
        Function<int(int)> func(bound);

        CHECK(func(5) == 15);
        CHECK(func(10) == 30);

        calc.base = 4;
        CHECK(func(5) == 20);
    }

    TEST_CASE("BindMember with multiple arguments")
    {
        Calculator calc;
        calc.base = 100;

        auto bound = BindMember<&Calculator::AddTwo>(&calc);
        Function<int(int, int)> func(bound);

        CHECK(func(10, 20) == 130);
        CHECK(func(5, 5) == 110);
    }

    TEST_CASE("BindMember with const object")
    {
        const Calculator calc{42};

        auto bound = BindMember<&Calculator::Multiply>(&calc);
        Function<int(int)> func(bound);

        CHECK(func(2) == 84);
        CHECK(func(10) == 420);
    }

    TEST_CASE("Custom buffer size - small buffer")
    {
        auto lambda = [](int x) { return x + 1; };
        Function<int(int), 16> func(lambda);

        CHECK(func.Valid());
        CHECK(func(5) == 6);
    }

    TEST_CASE("Custom buffer size - larger buffer")
    {
        // Create a lambda with lots of captures to test larger buffer
        int a = 1, b = 2, c = 3, d = 4;
        auto lambda = [a, b, c, d](int x) { return x + a + b + c + d; };

        Function<int(int), 64> func(lambda);

        CHECK(func.Valid());
        CHECK(func(10) == 20);
    }

    TEST_CASE("Function with pointer return type")
    {
        static int value = 42;
        auto lambda = []() -> int* { return &value; };
        Function<int*()> func(lambda);

        int* result = func();
        CHECK(result == &value);
        CHECK(*result == 42);
    }

    TEST_CASE("Function with reference parameter")
    {
        auto lambda = [](int& x) { x *= 2; };
        Function<void(int&)> func(lambda);

        int value = 10;
        func(value);
        CHECK(value == 20);
    }

    TEST_CASE("Function with const reference parameter")
    {
        auto lambda = [](const int& x) { return x * 2; };
        Function<int(const int&)> func(lambda);

        int value = 15;
        CHECK(func(value) == 30);
    }

    TEST_CASE("Chained move assignments")
    {
        auto lambda1 = [](int x) { return x * 2; };
        auto lambda2 = [](int x) { return x + 10; };
        auto lambda3 = [](int x) { return x - 5; };

        Function<int(int)> f1(lambda1);
        Function<int(int)> f2(lambda2);
        Function<int(int)> f3(lambda3);

        CHECK(f1(5) == 10);
        CHECK(f2(5) == 15);
        CHECK(f3(5) == 0);

        f3 = rsblMove(f2);
        f2 = rsblMove(f1);

        CHECK_FALSE(f1.Valid());
        CHECK(f2(5) == 10);
        CHECK(f3(5) == 15);
    }

    TEST_CASE("Function stores and invokes lambda correctly")
    {
        FunctorTracker::resetCounters();

        FunctorTracker tracker(100);
        Function<int(int)> func(rsblMove(tracker));

        CHECK(func(10) == 110);
        CHECK(func(20) == 120);
        CHECK(FunctorTracker::callOperatorCalls == 2);
    }

    TEST_CASE("Multiple Functions with same functor type")
    {
        auto lambda1 = [](int x) { return x * 2; };
        auto lambda2 = [](int x) { return x * 3; };

        Function<int(int)> func1(lambda1);
        Function<int(int)> func2(lambda2);

        CHECK(func1(5) == 10);
        CHECK(func2(5) == 15);
    }

    TEST_CASE("Function with bool return type")
    {
        auto lambda = [](int x) { return x > 10; };
        Function<bool(int)> func(lambda);

        CHECK(func(15) == true);
        CHECK(func(5) == false);
    }

    TEST_CASE("Empty function becomes valid after assignment")
    {
        Function<int(int)> func;
        CHECK_FALSE(func.Valid());

        auto lambda = [](int x) { return x + 1; };
        func = Function<int(int)>(lambda);

        CHECK(func.Valid());
        CHECK(func(5) == 6);
    }

    TEST_CASE("Valid function becomes invalid after move")
    {
        auto lambda = [](int x) { return x + 1; };
        Function<int(int)> func1(lambda);
        CHECK(func1.Valid());

        Function<int(int)> func2(rsblMove(func1));
        CHECK_FALSE(func1.Valid());
        CHECK(func2.Valid());
    }

    TEST_CASE("BindMember creates correct wrapper size")
    {
        Calculator calc;
        auto bound = BindMember<&Calculator::Add>(&calc);

        // Should fit in default buffer size
        Function<int(int)> func(bound);
        CHECK(func.Valid());
        CHECK(func(5) == 15);
    }

    TEST_CASE("Function with string return type")
    {
        // This tests if we can return non-trivial types
        // However, we should avoid using std::string if this is supposed to be STL-free
        // So let's test with a const char* instead
        auto lambda = [](int x) -> const char* {
            return x > 0 ? "positive" : "non-positive";
        };
        Function<const char*(int)> func(lambda);

        CHECK(std::string(func(5)) == "positive");
        CHECK(std::string(func(-1)) == "non-positive");
    }

    TEST_CASE("Lambda with large capture")
    {
        // Test that we can capture multiple values
        struct LargeCapture
        {
            int values[4] = {1, 2, 3, 4};

            int operator()(int x) const
            {
                return x + values[0] + values[1] + values[2] + values[3];
            }
        };

        LargeCapture capture;
        Function<int(int), 64> func(capture);

        CHECK(func(10) == 20);
    }
}
