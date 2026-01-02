// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "include/rsbl-thread.h"

#include <atomic>

using namespace rsbl;

TEST_SUITE("rsbl::Thread")
{
    TEST_CASE("Create and execute simple thread that succeeds")
    {
        std::atomic<bool> executed{false};

        auto thread_result = Thread::Create([&executed]() -> Result<> {
            executed.store(true, std::memory_order_release);
            return ResultCode::Success;
        });

        REQUIRE(thread_result);

        auto thread = rsblMove(thread_result.Value());

        // Thread should be active initially
        CHECK(thread.IsActive());

        // Join the thread
        auto join_result = thread.Join();
        CHECK(join_result);

        // Thread should no longer be active
        CHECK_FALSE(thread.IsActive());

        // Verify the thread executed
        CHECK(executed.load(std::memory_order_acquire));

        // Check the function result
        const auto& func_result = thread.GetFunctionResult();
        CHECK(func_result);
        CHECK(func_result.Code() == ResultCode::Success);
    }

    TEST_CASE("Thread function that fails with error text")
    {
        auto thread_result = Thread::Create([]() -> Result<> {
            return "Thread encountered an error";
        });

        REQUIRE(thread_result);
        auto thread = rsblMove(thread_result.Value());

        auto join_result = thread.Join();
        CHECK(join_result);

        // Check the function result shows failure
        const auto& func_result = thread.GetFunctionResult();
        CHECK_FALSE(func_result);
        CHECK(func_result.Code() == ResultCode::Failure);

        // Check we can retrieve the error text
        const char* error_text = thread.GetResultText();
        CHECK(error_text != nullptr);
        CHECK(std::string(error_text) == "Thread encountered an error");
    }

    TEST_CASE("Thread with computation")
    {
        std::atomic<int> result{0};

        auto thread_result = Thread::Create([&result]() -> Result<> {
            int sum = 0;
            for (int i = 1; i <= 100; ++i)
            {
                sum += i;
            }
            result.store(sum, std::memory_order_release);
            return ResultCode::Success;
        });

        REQUIRE(thread_result);
        auto thread = rsblMove(thread_result.Value());

        auto join_result = thread.Join();
        CHECK(join_result);

        // Verify computation result (sum of 1..100 = 5050)
        CHECK(result.load(std::memory_order_acquire) == 5050);

        const auto& func_result = thread.GetFunctionResult();
        CHECK(func_result);
    }

    TEST_CASE("Thread with sleep to test IsActive")
    {
        std::atomic<bool> started{false};

        auto thread_result = Thread::Create([&started]() -> Result<> {
            started.store(true, std::memory_order_release);
            Thread::ThreadSleep(100); // Sleep for 100ms
            return ResultCode::Success;
        });

        REQUIRE(thread_result);
        auto thread = rsblMove(thread_result.Value());

        // Thread should be active
        CHECK(thread.IsActive());

        // Wait a bit to ensure thread has started
        Thread::ThreadSleep(10);
        CHECK(started.load(std::memory_order_acquire));

        // Thread should still be active (sleeping)
        CHECK(thread.IsActive());

        // Join and verify completion
        auto join_result = thread.Join();
        CHECK(join_result);
        CHECK_FALSE(thread.IsActive());
    }

    TEST_CASE("JoinTimeout with successful completion")
    {
        auto thread_result = Thread::Create([]() -> Result<> {
            Thread::ThreadSleep(50); // Sleep for 50ms
            return ResultCode::Success;
        });

        REQUIRE(thread_result);
        auto thread = rsblMove(thread_result.Value());

        // Join with a timeout longer than the sleep
        auto join_result = thread.JoinTimeout(500); // 500ms timeout
        CHECK(join_result);
        CHECK_FALSE(thread.IsActive());

        const auto& func_result = thread.GetFunctionResult();
        CHECK(func_result);
    }

    TEST_CASE("JoinTimeout with timeout expiration")
    {
        auto thread_result = Thread::Create([]() -> Result<> {
            Thread::ThreadSleep(500); // Sleep for 500ms
            return ResultCode::Success;
        });

        REQUIRE(thread_result);
        auto thread = rsblMove(thread_result.Value());

        // Join with a timeout shorter than the sleep
        auto join_result = thread.JoinTimeout(50); // 50ms timeout
        CHECK_FALSE(join_result);
        CHECK(std::string(join_result.FailureText()) == "Thread join timeout");

        // Thread should still be active
        CHECK(thread.IsActive());

        // Now join properly to clean up
        auto final_join = thread.Join();
        CHECK(final_join);
    }

    TEST_CASE("Double join returns error")
    {
        auto thread_result = Thread::Create([]() -> Result<> {
            return ResultCode::Success;
        });

        REQUIRE(thread_result);
        auto thread = rsblMove(thread_result.Value());

        // First join should succeed
        auto join_result1 = thread.Join();
        CHECK(join_result1);

        // Second join should fail
        auto join_result2 = thread.Join();
        CHECK_FALSE(join_result2);
        CHECK(std::string(join_result2.FailureText()) == "Thread already joined");
    }

    TEST_CASE("Thread::ThreadYield executes without error")
    {
        // Just verify that Yield can be called
        Thread::ThreadYield();
        // No assertions needed, just verify it doesn't crash
    }

    TEST_CASE("Thread::GetCurrentThreadId returns valid ID")
    {
        uint64 main_thread_id = Thread::GetCurrentThreadId();
        CHECK(main_thread_id != 0);

        std::atomic<uint64> thread_id{0};

        auto thread_result = Thread::Create([&thread_id]() -> Result<> {
            thread_id.store(Thread::GetCurrentThreadId(), std::memory_order_release);
            return ResultCode::Success;
        });

        REQUIRE(thread_result);
        auto thread = rsblMove(thread_result.Value());

        auto join_result = thread.Join();
        CHECK(join_result);

        uint64 worker_thread_id = thread_id.load(std::memory_order_acquire);
        CHECK(worker_thread_id != 0);
        CHECK(worker_thread_id != main_thread_id);
    }

    TEST_CASE("Thread move constructor")
    {
        auto thread_result = Thread::Create([]() -> Result<> {
            Thread::ThreadSleep(50);
            return ResultCode::Success;
        });

        REQUIRE(thread_result);
        auto thread1 = rsblMove(thread_result.Value());

        // Move thread1 to thread2
        auto thread2 = rsblMove(thread1);

        // thread2 should be joinable
        auto join_result = thread2.Join();
        CHECK(join_result);

        const auto& func_result = thread2.GetFunctionResult();
        CHECK(func_result);
    }

    TEST_CASE("Thread move assignment")
    {
        auto thread_result1 = Thread::Create([]() -> Result<> {
            return ResultCode::Success;
        });

        auto thread_result2 = Thread::Create([]() -> Result<> {
            Thread::ThreadSleep(50);
            return "Test failure";
        });

        REQUIRE(thread_result1);
        REQUIRE(thread_result2);

        auto thread1 = rsblMove(thread_result1.Value());
        auto thread2 = rsblMove(thread_result2.Value());

        // Move assign thread2 to thread1
        // This should join thread1 first
        thread1 = rsblMove(thread2);

        // thread1 should now have thread2's task
        auto join_result = thread1.Join();
        CHECK(join_result);

        const auto& func_result = thread1.GetFunctionResult();
        CHECK_FALSE(func_result);
        CHECK(std::string(thread1.GetResultText()) == "Test failure");
    }

    TEST_CASE("Destructor joins thread automatically")
    {
        std::atomic<bool> executed{false};

        {
            auto thread_result = Thread::Create([&executed]() -> Result<> {
                Thread::ThreadSleep(50);
                executed.store(true, std::memory_order_release);
                return ResultCode::Success;
            });

            REQUIRE(thread_result);
            auto thread = rsblMove(thread_result.Value());

            // Let thread go out of scope without explicit join
        }

        // Thread should have been joined by destructor
        CHECK(executed.load(std::memory_order_acquire));
    }

    TEST_CASE("Multiple threads execute independently")
    {
        std::atomic<int> counter{0};

        auto thread1_result = Thread::Create([&counter]() -> Result<> {
            for (int i = 0; i < 100; ++i)
            {
                counter.fetch_add(1, std::memory_order_relaxed);
                Thread::ThreadYield();
            }
            return ResultCode::Success;
        });

        auto thread2_result = Thread::Create([&counter]() -> Result<> {
            for (int i = 0; i < 100; ++i)
            {
                counter.fetch_add(1, std::memory_order_relaxed);
                Thread::ThreadYield();
            }
            return ResultCode::Success;
        });

        REQUIRE(thread1_result);
        REQUIRE(thread2_result);

        auto thread1 = rsblMove(thread1_result.Value());
        auto thread2 = rsblMove(thread2_result.Value());

        auto join1 = thread1.Join();
        auto join2 = thread2.Join();

        CHECK(join1);
        CHECK(join2);
        CHECK(counter.load(std::memory_order_relaxed) == 200);
    }
}
