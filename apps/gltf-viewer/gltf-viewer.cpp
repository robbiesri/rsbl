// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include <rsbl-ptr.h>
#include <rsbl-window.h>

#include <CLI11.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/LogMacros.h>
#include <quill/Logger.h>
#include <quill/sinks/ConsoleSink.h>
#include <quill/sinks/RotatingFileSink.h>

#include <string>

void print_gltf_stats(const fastgltf::Asset& asset, quill::Logger* logger)
{
    LOG_INFO(logger, "");
    LOG_INFO(logger, "=== glTF File Statistics ===");
    LOG_INFO(logger, "");

    // Basic counts
    LOG_INFO(logger, "Scenes:      {}", asset.scenes.size());
    LOG_INFO(logger, "Nodes:       {}", asset.nodes.size());
    LOG_INFO(logger, "Meshes:      {}", asset.meshes.size());
    LOG_INFO(logger, "Materials:   {}", asset.materials.size());
    LOG_INFO(logger, "Textures:    {}", asset.textures.size());
    LOG_INFO(logger, "Images:      {}", asset.images.size());
    LOG_INFO(logger, "Buffers:     {}", asset.buffers.size());
    LOG_INFO(logger, "Animations:  {}", asset.animations.size());
    LOG_INFO(logger, "Skins:       {}", asset.skins.size());
    LOG_INFO(logger, "Cameras:     {}", asset.cameras.size());

    // Mesh details
    if (!asset.meshes.empty())
    {
        LOG_INFO(logger, "");
        LOG_INFO(logger, "=== Mesh Details ===");
        size_t total_primitives = 0;
        for (size_t i = 0; i < asset.meshes.size(); ++i)
        {
            const auto& mesh = asset.meshes[i];
            if (!mesh.name.empty())
            {
                LOG_INFO(logger,
                         "  Mesh {}: {} primitive(s) (name: {})",
                         i,
                         mesh.primitives.size(),
                         mesh.name);
            }
            else
            {
                LOG_INFO(logger, "  Mesh {}: {} primitive(s)", i, mesh.primitives.size());
            }
            total_primitives += mesh.primitives.size();
        }
        LOG_INFO(logger, "  Total primitives: {}", total_primitives);
    }

    // Material details
    if (!asset.materials.empty())
    {
        LOG_INFO(logger, "");
        LOG_INFO(logger, "=== Material Details ===");
        for (size_t i = 0; i < asset.materials.size(); ++i)
        {
            const auto& material = asset.materials[i];
            if (!material.name.empty())
            {
                LOG_INFO(logger, "  Material {}: {}", i, material.name);
            }
            else
            {
                LOG_INFO(logger, "  Material {}", i);
            }
        }
    }

    // Buffer sizes
    if (!asset.buffers.empty())
    {
        LOG_INFO(logger, "");
        LOG_INFO(logger, "=== Buffer Information ===");
        size_t total_bytes = 0;
        for (size_t i = 0; i < asset.buffers.size(); ++i)
        {
            const auto& buffer = asset.buffers[i];
            if (!buffer.name.empty())
            {
                LOG_INFO(
                    logger, "  Buffer {}: {} bytes (name: {})", i, buffer.byteLength, buffer.name);
            }
            else
            {
                LOG_INFO(logger, "  Buffer {}: {} bytes", i, buffer.byteLength);
            }
            total_bytes += buffer.byteLength;
        }
        LOG_INFO(logger,
                 "  Total buffer size: {:.2f} KB ({:.2f} MB)",
                 total_bytes / 1024.0,
                 total_bytes / (1024.0 * 1024.0));
    }

    LOG_INFO(logger, "");
}

int main(int argc, char** argv)
{
    // Initialize quill backend
    quill::Backend::start();

    // Create console sink with custom pattern (just the message)
    auto console_sink =
        quill::Frontend::create_or_get_sink<quill::ConsoleSink>("console_sink", []() {
            quill::ConsoleSinkConfig config;
            config.set_override_pattern_formatter_options(quill::PatternFormatterOptions{
                "%(time) %(thread_id) %(file_name):%(line_number) %(message)"});
            return config;
        }());

    // Create rotating file sink that rotates daily
    auto file_sink =
        quill::Frontend::create_or_get_sink<quill::RotatingFileSink>("logs/gltf_viewer.log", []() {
            quill::RotatingFileSinkConfig config;
            config.set_rotation_time_daily("00:00"); // Rotate at midnight
            config.set_max_backup_files(30);         // Keep 30 days of logs
            return config;
        }());

    // Create logger with both sinks
    quill::Logger* logger = quill::Frontend::create_or_get_logger(
        "gltf_viewer", {std::move(console_sink), std::move(file_sink)});

    CLI::App app("GLTF viewer");

    std::string file_path;
    app.add_option("-f,--file", file_path, "GLTF file path")->required()->check(CLI::ExistingFile);

    CLI11_PARSE(app, argc, argv);

    LOG_INFO(logger, "Loading glTF file: {}", file_path);

    // Create fastgltf parser
    fastgltf::Parser parser;

    // Determine file type
    auto data = fastgltf::GltfDataBuffer::FromPath(file_path);
    if (data.error() != fastgltf::Error::None)
    {
        LOG_ERROR(logger, "Failed to load file: {}", fastgltf::getErrorMessage(data.error()));
        return 1;
    }

    // Parse options
    constexpr auto gltfOptions =
        fastgltf::Options::LoadExternalBuffers | fastgltf::Options::LoadExternalImages;

    // Parse the glTF file
    auto asset =
        parser.loadGltf(data.get(), std::filesystem::path(file_path).parent_path(), gltfOptions);
    if (asset.error() != fastgltf::Error::None)
    {
        LOG_ERROR(logger, "Failed to parse glTF: {}", fastgltf::getErrorMessage(asset.error()));
        return 1;
    }

    LOG_INFO(logger, "Successfully loaded glTF file!");

    // Print statistics
    print_gltf_stats(asset.get(), logger);

    LOG_INFO(logger, "Starting window...");
    rsbl::UniquePtr<rsbl::Window> window;
    auto window_create_result = rsbl::Window::Create({640, 480});
    if (window_create_result)
    {
        LOG_INFO(logger, "Window created successfully!");
        window = rsblMove(window_create_result.Value());
    }
    else
    {
        LOG_ERROR(logger, "Failed to create window: {}", window_create_result.FailureText());
        return 1;
    }

    while (window->ProcessMessages() != rsbl::WindowMessageResult::Quit)
    {
        // do stuff?

        // TODO: check resize
        if (window->CheckResize())
        {
            LOG_INFO(logger, "Resized window caught by app!");
        }
    }

    return 0;
}