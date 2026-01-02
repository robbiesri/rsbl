// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include "rsbl-thread.h"

#include <windows.h>

#include <cstring> // For strncpy_s

namespace rsbl
{

Thread::Thread()
    : m_threadFunc()
    , m_result(ResultCode::Success)
    , m_failureText{0}
    , m_isActive(false)
    , m_joined(false)
    , m_platformData{nullptr}
{
}

Result<Thread> Thread::Create(Function<Result<>()>&& thread_func)
{
    Thread thread;
    thread.m_threadFunc = rsblMove(thread_func);
    thread.m_isActive.store(true, std::memory_order_release);

    auto platform_result = CreatePlatformThread(&thread);
    if (!platform_result)
    {
        thread.m_isActive.store(false, std::memory_order_release);
        return platform_result.FailureText();
    }

    thread.m_platformData = platform_result.Value();
    return rsblMove(thread);
}

Thread::~Thread()
{
    // Join if not already joined to ensure thread completes
    if (!m_joined && m_platformData.platform_handle != nullptr)
    {
        rsblUnused(Join());
        // TODO: log failure to join? We're fubar at this point
    }

    if (m_platformData.platform_handle != nullptr)
    {
        ::CloseHandle(static_cast<HANDLE>(m_platformData.platform_handle));
    }
}

Thread::Thread(Thread&& other) noexcept
    : m_threadFunc(rsblMove(other.m_threadFunc))
    , m_result(rsblMove(other.m_result))
    , m_isActive(other.m_isActive.load(std::memory_order_acquire))
    , m_joined(other.m_joined)
    , m_platformData(other.m_platformData)
{
    // Copy failure text
    std::memcpy(m_failureText, other.m_failureText, kMaxFailureTextLength);

    // Invalidate the moved-from thread
    other.m_platformData.platform_handle = nullptr;
    other.m_joined = true; // Prevent double-join
}

Thread& Thread::operator=(Thread&& other) noexcept
{
    if (this != &other)
    {
        // Join our current thread if needed
        if (!m_joined && m_platformData.platform_handle != nullptr)
        {
            // TODO: Log the join results
            rsblUnused(Join());
        }

        // Close our current handle
        if (m_platformData.platform_handle != nullptr)
        {
            ::CloseHandle(static_cast<HANDLE>(m_platformData.platform_handle));
        }

        // Move data from other
        m_threadFunc = rsblMove(other.m_threadFunc);
        m_result = rsblMove(other.m_result);
        m_isActive.store(other.m_isActive.load(std::memory_order_acquire),
                         std::memory_order_release);
        m_joined = other.m_joined;
        m_platformData = other.m_platformData;
        std::memcpy(m_failureText, other.m_failureText, kMaxFailureTextLength);

        // Invalidate the moved-from thread
        other.m_platformData.platform_handle = nullptr;
        other.m_joined = true;
    }
    return *this;
}

bool Thread::IsActive() const
{
    return m_isActive.load(std::memory_order_acquire);
}

Result<> Thread::Join()
{
    if (m_joined)
    {
        return "Thread already joined";
    }

    if (m_platformData.platform_handle == nullptr)
    {
        return "Invalid thread handle";
    }

    HANDLE handle = static_cast<HANDLE>(m_platformData.platform_handle);
    DWORD wait_result = ::WaitForSingleObject(handle, INFINITE);

    if (wait_result != WAIT_OBJECT_0)
    {
        return "Failed to join thread";
    }

    m_joined = true;
    return ResultCode::Success;
}

Result<> Thread::JoinTimeout(uint32 timeout_ms)
{
    if (m_joined)
    {
        return "Thread already joined";
    }

    if (m_platformData.platform_handle == nullptr)
    {
        return "Invalid thread handle";
    }

    HANDLE handle = static_cast<HANDLE>(m_platformData.platform_handle);
    DWORD wait_result = ::WaitForSingleObject(handle, static_cast<DWORD>(timeout_ms));

    if (wait_result == WAIT_TIMEOUT)
    {
        return "Thread join timeout";
    }

    if (wait_result != WAIT_OBJECT_0)
    {
        return "Failed to join thread";
    }

    m_joined = true;
    return ResultCode::Success;
}

const Result<>& Thread::GetFunctionResult() const
{
    return m_result;
}

const char* Thread::GetResultText() const
{
    return m_failureText;
}

void Thread::ThreadSleep(uint32 milliseconds)
{
    ::Sleep(static_cast<DWORD>(milliseconds));
}

// Don't call this Yield, will collide with winbase macro
void Thread::ThreadYield()
{
    // ::SwitchToThread();
    _mm_pause();
}

uint64 Thread::GetCurrentThreadId()
{
    return static_cast<uint64>(::GetCurrentThreadId());
}

void Thread::ThreadEntry()
{
    // Execute the user's function and store the result
    m_result = m_threadFunc();

    // Capture failure text if the result failed
    // We need to copy it because it's stored in thread-local storage
    if (!m_result)
    {
        const char* failure_text = m_result.FailureText();
        if (failure_text != nullptr)
        {
#if defined(_MSC_VER)
            strncpy_s(m_failureText, kMaxFailureTextLength, failure_text, _TRUNCATE);
#else
            std::strncpy(m_failureText, failure_text, kMaxFailureTextLength - 1);
            m_failureText[kMaxFailureTextLength - 1] = '\0';
#endif
        }
    }

    // Mark thread as no longer active
    m_isActive.store(false, std::memory_order_release);
}

Result<ThreadNativeData> Thread::CreatePlatformThread(Thread* thread_obj)
{
    // Heh, I can call private functions from this lambda!
    // Windows thread entry point - must match the signature expected by CreateThread
    auto PlatformThreadFunction = [](void* parameter) -> DWORD {
        Thread* thread_obj = static_cast<Thread*>(parameter);
        thread_obj->ThreadEntry();
        return 0;
    };

    DWORD thread_id = 0;
    HANDLE handle = ::CreateThread(nullptr,                // Default security attributes
                                   0,                      // Default stack size
                                   PlatformThreadFunction, // Thread function
                                   thread_obj,             // Thread parameter
                                   0,                      // Creation flags (0 = run immediately)
                                   &thread_id);            // Thread ID output

    if (handle == nullptr)
    {
        return "Failed to create thread";
    }

    ThreadNativeData data;
    data.platform_handle = handle;
    return data;
}

} // namespace rsbl
