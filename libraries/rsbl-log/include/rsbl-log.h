// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#pragma once

/**
 * By defining QUILL_DISABLE_NON_PREFIXED_MACROS before including LogMacros, we disable the
 * default 'LOG_' and then create our own macros using the global logger.
 * Usage borrowed from quill recommend usage examples
 */
#define QUILL_DISABLE_NON_PREFIXED_MACROS

// Expose quill headers as required by TODO.md
#include "quill/LogMacros.h"
#include "quill/Logger.h"

namespace rsbl
{

/**
 * @brief Initialize the rsbl logging system with console and rotating file sinks.
 *
 * This function sets up the quill backend, creates a console sink and a rotating file sink,
 * and initializes the global logger with both sinks.
 *
 * @param log_file_path Path to the rotating log file
 */
void LogInit(const char* log_file_path);

/**
 * @brief Global logger instance for convenient logging throughout the application.
 *
 * This logger is initialized by LogInit() and configured with both console
 * and rotating file sinks.
 */
extern quill::Logger* g_logger;

} // namespace rsbl

// Convenience macros that use the global logger to simplify usage
#define RSBL_LOG_TRACE_L3(fmt, ...) QUILL_LOG_TRACE_L3(rsbl::g_logger, fmt, __VA_ARGS__)
#define RSBL_LOG_TRACE_L2(fmt, ...) QUILL_LOG_TRACE_L2(rsbl::g_logger, fmt, __VA_ARGS__)
#define RSBL_LOG_TRACE_L1(fmt, ...) QUILL_LOG_TRACE_L1(rsbl::g_logger, fmt, __VA_ARGS__)
#define RSBL_LOG_DEBUG(fmt, ...) QUILL_LOG_DEBUG(rsbl::g_logger, fmt, __VA_ARGS__)
#define RSBL_LOG_INFO(fmt, ...) QUILL_LOG_INFO(rsbl::g_logger, fmt, __VA_ARGS__)
#define RSBL_LOG_WARNING(fmt, ...) QUILL_LOG_WARNING(rsbl::g_logger, fmt, __VA_ARGS__)
#define RSBL_LOG_ERROR(fmt, ...) QUILL_LOG_ERROR(rsbl::g_logger, fmt, __VA_ARGS__)
#define RSBL_LOG_CRITICAL(fmt, ...) QUILL_LOG_CRITICAL(rsbl::g_logger, fmt, __VA_ARGS__)
