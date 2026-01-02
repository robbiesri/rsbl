// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#pragma once

#include <rsbl-function.h>
#include <rsbl-int-types.h>
#include <rsbl-ptr.h>
#include <rsbl-result.h>

#include <atomic>

// TODO: Support thread priorities
// TODO: Thread-local storage helpers
// TODO: Thread naming for debugger
// TODO: CPU affinity control

namespace rsbl
{

// Opaque handle to platform-specific thread data
struct ThreadNativeData
{
    void* platform_handle;
};

class Thread
{
  public:
    // Factory method to create and start a thread with a function
    // The function will be executed on a new thread immediately
    // The function must return Result<> to report success/failure
    // Returns a UniquePtr to keep the thread object at a fixed memory location
    static Result<UniquePtr<Thread>> Create(Function<Result<>()>&& thread_func);

    // Destructor - will join if not already joined
    ~Thread();

    // Not moveable or copyable (thread is actively executing and accessing this object)
    Thread(Thread&&) = delete;
    Thread& operator=(Thread&&) = delete;
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    // Check if the thread is still actively processing the client function
    // Returns true if the thread is running, false if it has completed or hasn't started
    bool IsActive() const;

    // Wait for the thread to complete
    // Returns success if thread completes, failure on error
    Result<> Join();

    // Wait for the thread to complete with a timeout (in milliseconds)
    // Returns success if thread completes within timeout
    // Returns failure if timeout expires or error occurs
    Result<> JoinTimeout(uint32 timeout_ms);

    // Get the result from the thread function
    // Must be called after Join() or JoinTimeout() succeeds
    const Result<>& GetFunctionResult() const;

    // Get the failure text from the thread function (if it failed)
    // Returns pointer to failure text string, or nullptr if no failure
    const char* GetResultText() const;

    // Static utility functions for thread management

    // Sleep current thread for specified milliseconds
    static void ThreadSleep(uint32 milliseconds);

    // Yield current thread's time slice to other threads
    static void ThreadYield();

    // Get the current thread's ID as a uint64
    static uint64 GetCurrentThreadId();

  private:
    // Private constructor - use Create() factory method
    Thread();

    // Internal thread entry point that wraps the user function
    void ThreadEntry();

    // Platform-specific thread creation
    static Result<ThreadNativeData> CreatePlatformThread(Thread* thread_obj);

    // The user's function to execute on the thread
    Function<Result<>()> m_threadFunc;

    // Storage for the result returned by the thread function
    Result<> m_result{ResultCode::Success};

    // Storage for failure text from the thread
    // We need to copy the text since it comes from thread-local storage
    static constexpr uint32 kMaxFailureTextLength = 256;
    char m_failureText[kMaxFailureTextLength];

    // Atomic flag indicating if thread is currently running
    std::atomic<bool> m_isActive;

    // Flag to track if thread has been joined
    bool m_joined;

    // Platform-specific thread data
    ThreadNativeData m_platformData;
};

} // namespace rsbl
