# TODO

## General

[x] Create core library
[x] Create platform library
[x] Internal DynamicArray class
[x] File handling APIs
[x] Result API for error handling
[x] Assert implementation
[x] Window management system for platform
[ ] Barebones smart pointers
[ ] Create render library
[ ] Set up support for DX12 and Vulkan
[ ] Create simple app that loads GLTF, and renders in real time
[ ] Add IMGUI to window to give controls
[ ] asset manager

## Infrastructure

[x] Testing with doctest
[x] Split code into libraries and apps
[x] Subtrees for git dependencies
[x] Pull down ninja binary for building (helps Claude run from CLI)
[x] Put cmake, python, and ninja all into the same folder?
[x] Command line options lib - CLI11
[x] Use release zips instead of subtrees?
[ ] Cache release zips on personal S3 bucket?

## Render Architecture

[ ] Build command list in render thread, send to execution thread to convert to API commands
[ ] Execution thread converts to API commands, builds command buffer, and submits.
[ ] Shader compile threads?
[ ] Resource creation happens in app thread
[ ] App loads resources from file, then creates associated rendering resources + scene graph
[ ] App thread defers loading to async loader thread
[ ] App thread builds scene (graph), submits to render thread to execute

## General Systems

[ ] CPU parallel job system, similar to PS3 cell system
[ ] Task graph system?