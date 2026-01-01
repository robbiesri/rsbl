# TODO

## General

[x] Create core library
[x] Create platform library
[x] Internal DynamicArray class
[x] File handling APIs
[x] Result API for error handling
[x] Assert implementation
[x] Window management system for platform
[x] Barebones smart pointers
[ ] Add IMGUI to window to give controls
[ ] Create render library
[ ] Set up support for DX12 and Vulkan
[ ] Create simple app that loads GLTF, and renders in real time
[ ] asset manager

## Infrastructure

[x] Testing with doctest
[x] Split code into libraries and apps
[x] Subtrees for git dependencies
[x] Pull down ninja binary for building (helps Claude run from CLI)
[x] Put cmake, python, and ninja all into the same folder?
[x] Command line options lib - CLI11
[x] Use release zips instead of subtrees?
[x] Function alternative to std::function
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

## Thread Management

## Functions

[x] Simpler alternative to std::function
[x] Configurable compile-time size specification for internal storage
[x] Supports free functions, lambdas, and instance methods
[x] Move only, no copies
[x] Minimal includes
[x] Bind support not needed initially
[x] Easy to debug
[x] No STL except placement new (and internal move + forward)
[x] Allow implicit conversions in call operator (different parameter pack type vs function template)