// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#pragma once

#include "include/rsbl-result.h"
#include "rsbl-int-types.h"

namespace rsbl
{

enum class ResultCode : uint8
{
    Success,
    Failure,
};

namespace Internal
{
    // Per-thread failure text used by Result
    void SetFailureText(const char* text);
    const char* GetFailureText();

    // Clients can use any ReturnType without worrying about constructor collisions because of this
    // internal opaque type. 'Empty' results will use this.
    struct DefaultReturnType
    {
        uint8 x;
    };
} // namespace Internal

// Custom specialization of std::optional specifically for using as an error-handling return type

// TODO: default return type?
template <typename ReturnType = Internal::DefaultReturnType>
class [[nodiscard]] Result
{
  private:
    // Instead of keeping an instance of ReturnType, we can use this buffer to control construction
    // time with placement new (borrowed from std::optional)
    alignas(ReturnType) uint8 m_valueBuffer[sizeof(ReturnType)];

    // Default to success, for ReturnType construction simplification
    ResultCode m_code = ResultCode::Success;

  public:
    // We assume success when passed the ReturnType
    // Explicit was suggested by linter, not sure if it's worth the possibile usability issues
    explicit Result(const ReturnType& value)
    {
        new (m_valueBuffer) ReturnType(value);
    }
    explicit Result(ReturnType&& value)
    {
        new (m_valueBuffer) ReturnType(rsblMove(value));
    }

    // Lightweight code-only constructors. Success invokes default constructor
    explicit Result(ResultCode code)
        : m_code(code)
    {
        if (m_code == ResultCode::Success)
        {
            new (m_valueBuffer) ReturnType();
        }
    }

    // TODO: compile-time detection constructors (if certain pass/fail types are passed in as args)

    // Moves
    Result(Result&& other)
        : m_code(other.m_code)
    {
        if (m_code == ResultCode::Success)
        {
            // lol, pain
            new (m_valueBuffer)
                ReturnType(rsblMove(*reinterpret_cast<ReturnType*>(other.m_valueBuffer)));
        }
    }

    Result& operator=(Result&& other)
    {
        // TODO: do I even care about checking about assigning to it's own self?

        // If we are overwriting a valid Result, run destructor on ReturnType
        if (m_code == ResultCode::Success)
        {
            reinterpret_cast<ReturnType*>(m_valueBuffer)->~ReturnType();
        }

        m_code = other.m_code;

        if (m_code == ResultCode::Success)
        {
            new (m_valueBuffer)
                ReturnType(rsblMove(*reinterpret_cast<ReturnType*>(other.m_valueBuffer)));
        }

        // invalidate the moved-away-from Result
        other.m_code = ResultCode::Failure;

        return *this;
    }

    // TODO: move overload for Result<> -> Result<ReturnType>
    // TODO: move overload for Result<ReturnType> -> Result<> (preserve error + failure text)

    // Error text
    Result(const char* text)
        : m_code(ResultCode::Failure)
    {
        Internal::SetFailureText(text);
    }

    // No default constructor or copies
    Result() = delete;
    Result(const Result&) = delete;
    Result& operator=(const Result&) = delete;

    ~Result()
    {
        if (m_code == ResultCode::Success)
        {
            reinterpret_cast<ReturnType*>(m_valueBuffer)->~ReturnType();
        }
    }

    // TODO: bool operator?

    ResultCode Code() const
    {
        return m_code;
    }
    ReturnType& Value()
    {
        return *reinterpret_cast<ReturnType*>(m_valueBuffer);
    }
    const ReturnType& Value() const
    {
        return *reinterpret_cast<const ReturnType*>(m_valueBuffer);
    }

    // I could make this static, but I want it took like we are checking the Result instance for
    // the text, not some general error buffer
    const char* FailureText() const
    {
        return Internal::GetFailureText();
    }
};

} // namespace rsbl
