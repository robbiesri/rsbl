// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "include/rsbl-core.h"

using namespace rsbl;

// Test helper struct
struct TestObject
{
    int value = 0;

    TestObject(int v = 0)
        : value(v)
    {
    }

    void Increment()
    {
        value++;
    }

    int GetValue() const
    {
        return value;
    }

    int operator()(int x) const
    {
        return value + x;
    }
};

TEST_SUITE("rsbl::ReferenceWrapper")
{
    TEST_CASE("Construction from lvalue reference")
    {
        int x = 42;
        ReferenceWrapper<int> ref(x);

        CHECK(ref.Get() == 42);
    }

    TEST_CASE("Get() returns reference to wrapped object")
    {
        int x = 10;
        ReferenceWrapper<int> ref(x);

        // Modify through reference
        ref.Get() = 20;

        CHECK(x == 20);
        CHECK(ref.Get() == 20);
    }

    TEST_CASE("Implicit conversion to reference")
    {
        int x = 100;
        ReferenceWrapper<int> ref(x);

        // Should implicitly convert to int&
        int& y = ref;
        y = 200;

        CHECK(x == 200);
        CHECK(ref.Get() == 200);
    }

    TEST_CASE("Copy constructor")
    {
        int x = 5;
        ReferenceWrapper<int> ref1(x);
        ReferenceWrapper<int> ref2(ref1);

        CHECK(ref2.Get() == 5);

        // Both should refer to the same object
        ref2.Get() = 10;
        CHECK(x == 10);
        CHECK(ref1.Get() == 10);
    }

    TEST_CASE("Copy assignment")
    {
        int x = 5;
        int y = 10;
        ReferenceWrapper<int> ref1(x);
        ReferenceWrapper<int> ref2(y);

        ref2 = ref1;

        // ref2 should now refer to x
        CHECK(ref2.Get() == 5);
        ref2.Get() = 20;
        CHECK(x == 20);
        CHECK(y == 10); // y unchanged
    }

    TEST_CASE("Wrapping objects")
    {
        TestObject obj(42);
        ReferenceWrapper<TestObject> ref(obj);

        CHECK(ref.Get().value == 42);

        ref.Get().Increment();
        CHECK(obj.value == 43);
        CHECK(ref.Get().value == 43);
    }

    TEST_CASE("Const reference wrapper")
    {
        const int x = 100;
        ReferenceWrapper<const int> ref(x);

        CHECK(ref.Get() == 100);

        // Should be able to read but not modify
        const int& y = ref;
        CHECK(y == 100);
    }

    TEST_CASE("Call operator forwarding for callable objects")
    {
        TestObject obj(10);
        ReferenceWrapper<TestObject> ref(obj);

        // TestObject has operator()(int)
        CHECK(ref(5) == 15);
        CHECK(ref(10) == 20);
    }

    TEST_CASE("Call operator forwarding with const wrapper")
    {
        const TestObject obj(10);
        ReferenceWrapper<const TestObject> ref(obj);

        CHECK(ref(5) == 15);
    }

    TEST_CASE("Multiple wrappers to same object")
    {
        int x = 1;
        ReferenceWrapper<int> ref1(x);
        ReferenceWrapper<int> ref2(x);
        ReferenceWrapper<int> ref3(x);

        ref1.Get() += 10;
        CHECK(x == 11);
        CHECK(ref2.Get() == 11);
        CHECK(ref3.Get() == 11);

        ref2.Get() *= 2;
        CHECK(x == 22);
        CHECK(ref1.Get() == 22);
        CHECK(ref3.Get() == 22);
    }

    TEST_CASE("Reference wrapper in array")
    {
        int a = 1, b = 2, c = 3;
        ReferenceWrapper<int> refs[] = {a, b, c};

        CHECK(refs[0].Get() == 1);
        CHECK(refs[1].Get() == 2);
        CHECK(refs[2].Get() == 3);

        refs[1].Get() = 20;
        CHECK(b == 20);
    }

    TEST_CASE("Type alias")
    {
        int x = 42;
        ReferenceWrapper<int> ref(x);

        // Check that the type alias works
        ReferenceWrapper<int>::type value = ref.Get();
        CHECK(value == 42);
    }
}

TEST_SUITE("rsbl::Ref")
{
    TEST_CASE("Creates ReferenceWrapper from lvalue")
    {
        int x = 42;
        auto ref = Ref(x);

        CHECK(ref.Get() == 42);

        ref.Get() = 100;
        CHECK(x == 100);
    }

    TEST_CASE("Works with objects")
    {
        TestObject obj(50);
        auto ref = Ref(obj);

        CHECK(ref.Get().value == 50);

        ref.Get().Increment();
        CHECK(obj.value == 51);
    }

    TEST_CASE("Can be stored and copied")
    {
        int x = 10;
        auto ref1 = Ref(x);
        auto ref2 = ref1;

        ref2.Get() = 20;
        CHECK(x == 20);
        CHECK(ref1.Get() == 20);
    }

    TEST_CASE("Used in function parameter passing")
    {
        auto increment = [](ReferenceWrapper<int> ref) {
            ref.Get()++;
        };

        int x = 5;
        increment(Ref(x));
        CHECK(x == 6);
    }

    TEST_CASE("Multiple Ref calls on same object")
    {
        int x = 1;
        auto ref1 = Ref(x);
        auto ref2 = Ref(x);

        ref1.Get() = 10;
        CHECK(ref2.Get() == 10);
    }
}

TEST_SUITE("rsbl::CRef")
{
    TEST_CASE("Creates const ReferenceWrapper")
    {
        int x = 42;
        auto ref = CRef(x);

        CHECK(ref.Get() == 42);
    }

    TEST_CASE("Works with const objects")
    {
        const TestObject obj(100);
        auto ref = CRef(obj);

        CHECK(ref.Get().value == 100);
        CHECK(ref.Get().GetValue() == 100);
    }

    TEST_CASE("Can be stored and copied")
    {
        int x = 10;
        auto ref1 = CRef(x);
        auto ref2 = ref1;

        CHECK(ref2.Get() == 10);
    }

    TEST_CASE("Used in function parameter passing")
    {
        auto getValue = [](ReferenceWrapper<const int> ref) {
            return ref.Get();
        };

        int x = 42;
        CHECK(getValue(CRef(x)) == 42);
    }

    TEST_CASE("Call operator with const callable")
    {
        const TestObject obj(20);
        auto ref = CRef(obj);

        CHECK(ref(5) == 25);
    }
}

TEST_SUITE("rsbl::UnwrapReference")
{
    TEST_CASE("Non-wrapper types remain unchanged")
    {
        static_assert(sizeof(UnwrapReferenceType<int>) == sizeof(int), "");
        static_assert(sizeof(UnwrapReferenceType<double>) == sizeof(double), "");
    }

    TEST_CASE("ReferenceWrapper unwraps to reference")
    {
        int x = 42;
        ReferenceWrapper<int> ref(x);

        UnwrapReferenceType<ReferenceWrapper<int>> unwrapped = ref;
        unwrapped = 100;

        CHECK(x == 100);
    }

    TEST_CASE("Const ReferenceWrapper unwraps to const reference")
    {
        int x = 42;
        ReferenceWrapper<const int> ref(x);

        UnwrapReferenceType<ReferenceWrapper<const int>> unwrapped = ref;
        CHECK(unwrapped == 42);
    }
}

TEST_SUITE("ReferenceWrapper with Function")
{
    TEST_CASE("Pass by reference using Ref")
    {
        int counter = 0;

        auto incrementor = [](int& x) {
            x++;
        };

        // Simulate storing in a container that requires copy
        ReferenceWrapper<int> refWrapper = Ref(counter);

        // Use it later
        incrementor(refWrapper);
        CHECK(counter == 1);

        incrementor(refWrapper);
        CHECK(counter == 2);
    }

    TEST_CASE("Store multiple references in array")
    {
        int a = 1, b = 2, c = 3;
        auto refs = {Ref(a), Ref(b), Ref(c)};

        for (auto ref : refs)
        {
            ref.Get() *= 10;
        }

        CHECK(a == 10);
        CHECK(b == 20);
        CHECK(c == 30);
    }

    TEST_CASE("Reference wrapper preserves object identity")
    {
        TestObject obj(5);
        auto ref1 = Ref(obj);
        auto ref2 = ref1;

        // Both should point to the same object
        ref1.Get().value = 10;
        CHECK(ref2.Get().value == 10);
        CHECK(obj.value == 10);

        ref2.Get().value = 20;
        CHECK(ref1.Get().value == 20);
        CHECK(obj.value == 20);
    }

    TEST_CASE("Callable object through wrapper")
    {
        auto lambda = [](int x, int y) {
            return x + y;
        };

        auto ref = Ref(lambda);
        CHECK(ref(5, 10) == 15);
        CHECK(ref(100, 200) == 300);
    }

    TEST_CASE("Mutable lambda through wrapper")
    {
        int counter = 0;
        auto lambda = [counter]() mutable {
            return ++counter;
        };

        auto ref = Ref(lambda);
        CHECK(ref() == 1);
        CHECK(ref() == 2);
        CHECK(ref() == 3);
    }
}
