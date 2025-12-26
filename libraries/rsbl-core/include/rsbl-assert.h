// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#pragma once

// Inspired by https://www.foonathan.net/2016/09/assertions/ and
// http://cnicholson.net/2009/02/stupid-c-tricks-adventures-in-assert/, along with some cool
// debugbreak stuff from https://github.com/scottt/debugbreak

#if !defined(RSBL_ASSERTS_DISABLED)
    #define RSBL_ASSERTS_ENABLED
#endif

namespace rsbl
{
namespace Assert
{
    // Default handler halts, but user can set handler that will continue
    enum class FailureBehavior
    {
        Halt,
        Continue,
    };

    typedef FailureBehavior (*Handler)(const char* condition,
                                       const char* msg,
                                       const char* file,
                                       int line);

    Handler GetHandler();
    void SetHandler(Handler newHandler);

    // User can chain to default handler if needed
    Handler GetDefaultHandler();

    FailureBehavior ReportFailure(const char* condition,
                                  const char* file,
                                  int line,
                                  const char* msg);
} // namespace Assert
} // namespace rsbl

// great research on different break invocations from https://github.com/scottt/debugbreak
#if defined(_MSC_VER)
    #define rsblDebugBreak() (__debugbreak())
#elif defined(__aarch64__)
    #define rsblDebugBreak() (__asm__ volatile(".inst 0xd4200000"))
#elif defined(__arm__) && !defined(__thumb__)
    #define rsblDebugBreak() (__asm__ volatile(".inst 0xe7f001f0"))
#else
// TODO: support additional platforms
static_assert(false, "Missing platform implementation for assert break");
#endif

// sizeof _does not_ evaluate a given expression, which allows to omit code-gen when
// asserts are not enabled! the (void) is to throw away the results to prevent unused warnings
// idea to use (void)sizeof comes from
// http://cnicholson.net/2009/02/stupid-c-tricks-adventures-in-assert/
#define rsblUnused(x) \
    do \
    { \
        (void)sizeof(x); \
    } while (0)

// We always wrap condition in parentheses to guarantee correct eval
// do-while(0) allows us to have a multi-statement operation that _could_
// be used inside an if-statement without braces.
// Note: double paren around condition to avoid clang-tidy warning

#if defined(RSBL_ASSERTS_ENABLED)
    #define rsblAssert(condition) \
        do \
        { \
            if (!((condition))) \
            { \
                if (rsbl::Assert::ReportFailure(#condition, __FILE__, __LINE__, nullptr) == \
                    rsbl::Assert::FailureBehavior::Halt) \
                    rsblDebugBreak(); \
            } \
        } while (0)

    #define rsblAssertMsg(condition, msg) \
        do \
        { \
            if (!((condition))) \
            { \
                if (rsbl::Assert::ReportFailure(#condition, __FILE__, __LINE__, (msg)) == \
                    rsbl::Assert::FailureBehavior::Halt) \
                    rsblDebugBreak(); \
            } \
        } while (0)

#else // defined(RSBL_ASSERTS_ENABLED)

    #define rsblAssert(condition) \
        do \
        { \
            rsblUnused(condition); \
        } while (0)

    #define rsblAssertMsg(condition, msg) \
        do \
        { \
            rsblUnused(condition); \
            rsblUnused(msg); \
        } while (0)

#endif // defined(RSBL_ASSERTS_ENABLED)
