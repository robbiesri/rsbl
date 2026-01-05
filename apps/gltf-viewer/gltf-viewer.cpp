// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include <rsbl-ga.h>
#include <rsbl-log.h>
#include <rsbl-ptr.h>
#include <rsbl-window.h>

#include <CLI11.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>

#include <string>

void print_gltf_stats(const fastgltf::Asset& asset)
{
    RSBL_LOG_INFO("");
    RSBL_LOG_INFO("=== glTF File Statistics ===");
    RSBL_LOG_INFO("");

    // Basic counts
    RSBL_LOG_INFO("Scenes:      {}", asset.scenes.size());
    RSBL_LOG_INFO("Nodes:       {}", asset.nodes.size());
    RSBL_LOG_INFO("Meshes:      {}", asset.meshes.size());
    RSBL_LOG_INFO("Materials:   {}", asset.materials.size());
    RSBL_LOG_INFO("Textures:    {}", asset.textures.size());
    RSBL_LOG_INFO("Images:      {}", asset.images.size());
    RSBL_LOG_INFO("Buffers:     {}", asset.buffers.size());
    RSBL_LOG_INFO("Animations:  {}", asset.animations.size());
    RSBL_LOG_INFO("Skins:       {}", asset.skins.size());
    RSBL_LOG_INFO("Cameras:     {}", asset.cameras.size());

    // Mesh details
    if (!asset.meshes.empty())
    {
        RSBL_LOG_INFO("");
        RSBL_LOG_INFO("=== Mesh Details ===");
        size_t total_primitives = 0;
        for (size_t i = 0; i < asset.meshes.size(); ++i)
        {
            const auto& mesh = asset.meshes[i];
            if (!mesh.name.empty())
            {
                RSBL_LOG_INFO(
                    "  Mesh {}: {} primitive(s) (name: {})", i, mesh.primitives.size(), mesh.name);
            }
            else
            {
                RSBL_LOG_INFO("  Mesh {}: {} primitive(s)", i, mesh.primitives.size());
            }
            total_primitives += mesh.primitives.size();
        }
        RSBL_LOG_INFO("  Total primitives: {}", total_primitives);
    }

    // Material details
    if (!asset.materials.empty())
    {
        RSBL_LOG_INFO("");
        RSBL_LOG_INFO("=== Material Details ===");
        for (size_t i = 0; i < asset.materials.size(); ++i)
        {
            const auto& material = asset.materials[i];
            if (!material.name.empty())
            {
                RSBL_LOG_INFO("  Material {}: {}", i, material.name);
            }
            else
            {
                RSBL_LOG_INFO("  Material {}", i);
            }
        }
    }

    // Buffer sizes
    if (!asset.buffers.empty())
    {
        RSBL_LOG_INFO("");
        RSBL_LOG_INFO("=== Buffer Information ===");
        size_t total_bytes = 0;
        for (size_t i = 0; i < asset.buffers.size(); ++i)
        {
            const auto& buffer = asset.buffers[i];
            if (!buffer.name.empty())
            {
                RSBL_LOG_INFO(
                    "  Buffer {}: {} bytes (name: {})", i, buffer.byteLength, buffer.name);
            }
            else
            {
                RSBL_LOG_INFO("  Buffer {}: {} bytes", i, buffer.byteLength);
            }
            total_bytes += buffer.byteLength;
        }
        RSBL_LOG_INFO("  Total buffer size: {:.2f} KB ({:.2f} MB)",
                      total_bytes / 1024.0,
                      total_bytes / (1024.0 * 1024.0));
    }

    RSBL_LOG_INFO("");
}

int main(int argc, char** argv)
{
    rsbl_log_init("logs/gltf_viewer.log");

    CLI::App app("GLTF viewer");

    std::string file_path;
    app.add_option("-f,--file", file_path, "GLTF file path")->required()->check(CLI::ExistingFile);

    CLI11_PARSE(app, argc, argv);

    RSBL_LOG_INFO("Loading glTF file: {}", file_path);

    // Create fastgltf parser
    fastgltf::Parser parser;

    // Determine file type
    auto data = fastgltf::GltfDataBuffer::FromPath(file_path);
    if (data.error() != fastgltf::Error::None)
    {
        RSBL_LOG_ERROR("Failed to load file: {}", fastgltf::getErrorMessage(data.error()));
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
        RSBL_LOG_ERROR("Failed to parse glTF: {}", fastgltf::getErrorMessage(asset.error()));
        return 1;
    }

    RSBL_LOG_INFO("Successfully loaded glTF file!");

    // Print statistics
    print_gltf_stats(asset.get());

    RSBL_LOG_INFO("Starting window...");
    rsbl::UniquePtr<rsbl::Window> window;
    auto window_create_result = rsbl::Window::Create({640, 480});
    if (window_create_result)
    {
        RSBL_LOG_INFO("Window created successfully!");
        window = rsblMove(window_create_result.Value());
    }
    else
    {
        RSBL_LOG_ERROR("Failed to create window: {}", window_create_result.FailureText());
        return 1;
    }

    rsbl::gaDevice* dx12_device = nullptr;
    rsbl::gaDevice* vulkan_device = nullptr;

    rsbl::gaDeviceCreateInfo create_info_dx12{};
    create_info_dx12.backend = rsbl::gaBackend::DX12;
    if (auto dx12_device_result = rsbl::GaCreateDevice(create_info_dx12))
    {
        dx12_device = dx12_device_result.Value();
        RSBL_LOG_INFO("DX12 device successfully created");
    }
    else
    {
        RSBL_LOG_WARNING("Failed to create DX12 device: {}", dx12_device_result.FailureText());
    }

    rsbl::gaDeviceCreateInfo create_info_vulkan{};
    create_info_vulkan.backend = rsbl::gaBackend::Vulkan;
    if (auto vulkan_device_result = rsbl::GaCreateDevice(create_info_vulkan))
    {
        vulkan_device = vulkan_device_result.Value();
        RSBL_LOG_INFO("Vulkan device successfully created");
    }
    else
    {
        RSBL_LOG_WARNING("Failed to create Vulkan device: {}", vulkan_device_result.FailureText());
    }

    while (window->ProcessMessages() != rsbl::WindowMessageResult::Quit)
    {
        // do stuff?

        // TODO: check resize
        if (window->CheckResize())
        {
            RSBL_LOG_INFO("Resized window caught by app!");
        }
    }

    rsbl::GaDestroyDevice(dx12_device);
    rsbl::GaDestroyDevice(vulkan_device);

    RSBL_LOG_INFO("Window closed, shutting down!");

    return 0;
}