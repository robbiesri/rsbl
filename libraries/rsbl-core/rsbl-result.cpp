// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include "include/rsbl-result.h"

#include <string>

// TODO: test sizes of Result with different ReturnTypes

// TODO: eventually replace with our own string class. Alternatively, char* buffer?
static thread_local std::string s_failureTextStr;

namespace rsbl::Internal
{
void SetFailureText(const char* text)
{
    if (text == s_failureTextStr.c_str())
    {
        // is this possible?
        return;
    }

    // rsblAssert(text != nullptr);

    s_failureTextStr = text;
}

const char* GetFailureText()
{
    return s_failureTextStr.c_str();
}
} // namespace rsbl::Internal