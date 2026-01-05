// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include "rsbl-log.h"

#include <cstdio>

// Simple test program that verifies rsbl-log compiles and links correctly
// Full macro testing will be done in actual application usage

int main()
{
    // Initialize the logger
    rsbl_log_init("test_rsbl_log.log");

    if (g_rsbl_logger == nullptr)
    {
        printf("FAILED: Logger initialization returned nullptr\n");
        return 1;
    }

    // Verify logger is accessible
    if (g_rsbl_logger != nullptr)
    {
        printf("PASSED: Logger initialized successfully\n");
    }

    RSBL_LOG_INFO("What?");

    printf("rsbl-log library test completed successfully\n");
    return 0;
}
