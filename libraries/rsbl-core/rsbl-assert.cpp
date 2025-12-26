
#include "rsbl-assert.h"

#include <cstdio>
#include <cstdlib>

#if defined(_MSC_VER)
    #include <windows.h>

    #pragma comment(lib, "user32.lib")
#endif

namespace
{
void DebuggerOutput(const char* error_buffer)
{
#if defined(_MSC_VER)
    OutputDebugStringA(error_buffer);
#endif
}

// TODO: bring up debug dialog box on windows. Does this not happen by default?

// TODO: replace snprintf/fprintf with other print tooling?
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/crt-debugging-techniques
rsbl::Assert::FailureBehavior DefaultHandler(const char* condition,
                                             const char* msg,
                                             const char* file,
                                             const int line)
{

    char error_buffer[2048];
    sprintf_s(error_buffer,
              "%s(%d): Assert Failure: '%s' %s\n",
              file,
              line,
              condition != nullptr ? condition : "",
              msg != nullptr ? msg : "");

    fprintf(stderr, "%s", error_buffer);
    DebuggerOutput(error_buffer);

    return rsbl::Assert::FailureBehavior::Halt;
}

rsbl::Assert::Handler s_assertHandler = &DefaultHandler;
} // namespace

rsbl::Assert::Handler rsbl::Assert::GetHandler()
{
    return s_assertHandler;
}

void rsbl::Assert::SetHandler(rsbl::Assert::Handler new_handler)
{
    rsblAssert(new_handler != nullptr);
    s_assertHandler = new_handler;
}

rsbl::Assert::Handler rsbl::Assert::GetDefaultHandler()
{
    return &DefaultHandler;
}

rsbl::Assert::FailureBehavior rsbl::Assert::ReportFailure(const char* condition,
                                                          const char* file,
                                                          const int line,
                                                          const char* msg)
{
    return s_assertHandler(condition, msg, file, line);
}