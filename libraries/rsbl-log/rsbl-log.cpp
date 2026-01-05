// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include "rsbl-log.h"

// Include quill backend header as required by TODO.md
#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/sinks/ConsoleSink.h"
#include "quill/sinks/RotatingFileSink.h"

// Global static logger instance owned by wrapper library
quill::Logger* g_rsbl_logger = nullptr;

void rsbl_log_init(const char* log_file_path)
{
    // Start the backend thread
    quill::Backend::start();

    // Create console sink with custom pattern format
    auto console_sink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>(
        "rsbl_console_sink",
        []()
        {
            quill::ConsoleSinkConfig cfg;
            cfg.set_override_pattern_formatter_options(
                quill::PatternFormatterOptions{
                    "%(time) %(thread_id) %(file_name):%(line_number) %(message)",
                    "%H:%M:%S.%Qns",
                    quill::Timezone::LocalTime});
            return cfg;
        }());

    // Create rotating file sink
    auto rotating_file_sink = quill::Frontend::create_or_get_sink<quill::RotatingFileSink>(
        log_file_path,
        []()
        {
            quill::RotatingFileSinkConfig cfg;
            cfg.set_open_mode('w');
            cfg.set_rotation_max_file_size(1024 * 1024 * 10);  // 10 MB rotation size
            cfg.set_max_backup_files(5);                        // Keep up to 5 backup files
            cfg.set_overwrite_rolled_files(true);               // Overwrite old files when limit reached
            cfg.set_filename_append_option(quill::FilenameAppendOption::StartDateTime);
            return cfg;
        }());

    // Create logger with both console and rotating file sinks
    g_rsbl_logger = quill::Frontend::create_or_get_logger(
        "rsbl_root",
        {console_sink, rotating_file_sink});
}
