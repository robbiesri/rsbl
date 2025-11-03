// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include "../../external/CLI11/CLI11.hpp"

#include <cstdio>
#include <string>

#include <CLI11.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>

void print_gltf_stats(const fastgltf::Asset& asset)
{
    printf("\n");
    printf("=== glTF File Statistics ===\n");
    printf("\n");

    // Basic counts
    printf("Scenes:      %zu\n", asset.scenes.size());
    printf("Nodes:       %zu\n", asset.nodes.size());
    printf("Meshes:      %zu\n", asset.meshes.size());
    printf("Materials:   %zu\n", asset.materials.size());
    printf("Textures:    %zu\n", asset.textures.size());
    printf("Images:      %zu\n", asset.images.size());
    printf("Buffers:     %zu\n", asset.buffers.size());
    printf("Animations:  %zu\n", asset.animations.size());
    printf("Skins:       %zu\n", asset.skins.size());
    printf("Cameras:     %zu\n", asset.cameras.size());

    // Mesh details
    if (!asset.meshes.empty())
    {
        printf("\n");
        printf("=== Mesh Details ===\n");
        size_t total_primitives = 0;
        for (size_t i = 0; i < asset.meshes.size(); ++i)
        {
            const auto& mesh = asset.meshes[i];
            printf("  Mesh %zu: %zu primitive(s)", i, mesh.primitives.size());
            if (!mesh.name.empty())
            {
                printf(" (name: %s)", mesh.name.c_str());
            }
            printf("\n");
            total_primitives += mesh.primitives.size();
        }
        printf("  Total primitives: %zu\n", total_primitives);
    }

    // Material details
    if (!asset.materials.empty())
    {
        printf("\n");
        printf("=== Material Details ===\n");
        for (size_t i = 0; i < asset.materials.size(); ++i)
        {
            const auto& material = asset.materials[i];
            printf("  Material %zu", i);
            if (!material.name.empty())
            {
                printf(": %s", material.name.c_str());
            }
            printf("\n");
        }
    }

    // Buffer sizes
    if (!asset.buffers.empty())
    {
        printf("\n");
        printf("=== Buffer Information ===\n");
        size_t total_bytes = 0;
        for (size_t i = 0; i < asset.buffers.size(); ++i)
        {
            const auto& buffer = asset.buffers[i];
            printf("  Buffer %zu: %zu bytes", i, buffer.byteLength);
            if (!buffer.name.empty())
            {
                printf(" (name: %s)", buffer.name.c_str());
            }
            printf("\n");
            total_bytes += buffer.byteLength;
        }
        printf("  Total buffer size: %.2f KB (%.2f MB)\n",
               total_bytes / 1024.0,
               total_bytes / (1024.0 * 1024.0));
    }

    printf("\n");
}

int main(int argc, char** argv)
{
    CLI::App app("GLTF viewer");

    std::string file_path;
    app.add_option("-f,--file", file_path, "GLTF file path")->required()->check(CLI::ExistingFile);

    CLI11_PARSE(app, argc, argv);

    printf("Loading glTF file: %s\n", file_path.c_str());

    // Create fastgltf parser
    fastgltf::Parser parser;

    // Determine file type
    auto data = fastgltf::GltfDataBuffer::FromPath(file_path);
    if (data.error() != fastgltf::Error::None)
    {
        fprintf(stderr,
                "ERROR: Failed to load file: %s\n",
                fastgltf::getErrorMessage(data.error()).data());
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
        fprintf(stderr,
                "ERROR: Failed to parse glTF: %s\n",
                fastgltf::getErrorMessage(asset.error()).data());
        return 1;
    }

    printf("Successfully loaded glTF file!\n");

    // Print statistics
    print_gltf_stats(asset.get());

    return 0;
}