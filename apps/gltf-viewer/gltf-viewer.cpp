// Copyright 2025 Robert Srinivasiah
// Licensed under the MIT License, see the LICENSE file for more info

#include "../../external/CLI11/CLI11.hpp"

#include <cstdio>
#include <string>

#include <CLI11.hpp>

int main(int argc, char** argv)
{
    CLI::App app("GLTF viewer");

    std::string file_path;
    app.add_option("-f,--file", file_path, "GLTF file path")->required()->check(CLI::ExistingFile);

    CLI11_PARSE(app, argc, argv);

    printf("Hello graphics world!\n");
    return 0;
}