# Copyright 2025 Robert Srinivasiah
# Licensed under the MIT License, see the LICENSE file for more info

set(APP_NAME gltf-viewer)

list(APPEND SRC_FILES
        gltf-viewer.cpp
)

add_executable(${APP_NAME}
        ${SRC_FILES}
)

target_link_libraries(${APP_NAME}
        PUBLIC
        rsbl-platform
        rsbl-ga
        fastgltf
        quill::quill
)

target_include_directories(${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/external/CLI11)
