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
[ ] Add logging to libraries!
[ ] Cache release zips on personal S3 bucket?
[ ] Profiling infrastructure (Tracy?)

## General Systems

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

## Thread Management

[x] Simpler alternative to std::thread
[x] Minimal includes, avoid heavy STL headers
[x] Build on rsbl::Function, rsbl::Result and std::atomic
[x] Disallow moves and copies
[x] Static creation function that return `rsbl::UniquePtr<rsbl::Thread>`
[x] Support sleep + yield
[x] Method to query if the thread is still actively processing the client function
[x] Join method that can take optional timeout

## Task/Job System

[ ] Read existing literature to figure out patterns
[ ] Fibers vs threads?
[ ] Express dependencies between tasks
[ ] High, medium, low priority tasks
[ ] Allow for 'fast' task generation with task IDs to parcel out parallel friendly tasks
[ ] Visualizer for task hierarchy (Chrome?)

## Graphics API Abstraction

[ ] Mix of free standing function + command list builders
[ ] Free standing APIs to create resources - textures, buffers, shaders, render targets, render passes, devices
[ ] Free standing APIs to reset + submit command lists
[ ] Command list APIs to bind resources, draws + dispatch, and present
[ ] Command lists are API opaque, and translated at submit time
[ ] Command lists can be processed on 'submit' thread, or in main thread for debugging
[ ] Resources can be created across multiple devices
[ ] Submits can process across multiple devices

## Render Architecture

[ ] Build command list in render thread, send to execution thread to convert to API commands
[ ] Execution thread converts to API commands, builds command buffer, and submits.
[ ] Shader compile threads?
[ ] Resource creation happens in app thread
[ ] App loads resources from file, then creates associated rendering resources + scene graph
[ ] App thread defers loading to async loader thread
[ ] App thread builds scene (graph), submits to render thread to execute

### Render Task Graph

[ ] Similar to task system, but handle render transitions (build on top of task system?)
