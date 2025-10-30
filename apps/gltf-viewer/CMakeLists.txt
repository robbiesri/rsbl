# Copyright 2025 Robert Srinivasiah
# Licensed under the MIT License, see the LICENSE file for more info

list(APPEND SRC_FILES
        gltf-viewer.cpp
)

add_executable(gltf-viewer
        ${SRC_FILES}
)

# TODO: link the project libraries!