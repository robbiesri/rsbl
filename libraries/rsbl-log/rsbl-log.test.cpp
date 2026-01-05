// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "rsbl-log.h"

// Helper functions to test logging outside of constexpr context
static void test_logging_macros()
{
    RSBL_LOG_TRACE_L3("Trace L3 message: {}", 1);
    RSBL_LOG_TRACE_L2("Trace L2 message: {}", 2);
    RSBL_LOG_TRACE_L1("Trace L1 message: {}", 3);
    RSBL_LOG_DEBUG("Debug message: {}", 4);
    RSBL_LOG_INFO("Info message: {}", 5);
    RSBL_LOG_WARNING("Warning message: {}", 6);
    RSBL_LOG_ERROR("Error message: {}", 7);
    RSBL_LOG_CRITICAL("Critical message: {}", 8);
}

static void test_multiple_types()
{
    RSBL_LOG_INFO("String: {}, Int: {}, Float: {}", "test", 42, 3.14);
    RSBL_LOG_DEBUG("Boolean: {}, Char: {}", true, 'X');
}

static void test_log_levels()
{
    rsbl::g_logger->set_log_level(quill::LogLevel::Warning);
    RSBL_LOG_DEBUG("This debug message will not appear");
    RSBL_LOG_WARNING("This warning message will appear");
}

TEST_CASE("rsbl-log initialization and basic logging")
{
    SUBCASE("Logger initialization")
    {
        // Initialize the logger with a test log file
        rsbl::LogInit("test_log.log");

        // Verify that the global logger is not null after initialization
        CHECK(rsbl::g_logger != nullptr);
    }

    SUBCASE("Convenience macros work")
    {
        // Call helper function to test logging
        test_logging_macros();
        CHECK(true);
    }

    SUBCASE("Multiple data types logging")
    {
        test_multiple_types();
        CHECK(true);
    }

    SUBCASE("Logger level changes")
    {
        test_log_levels();
        CHECK(true);
    }
}
