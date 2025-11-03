# TODO

## General

[x] Split code into libraries and apps
[x] Create core library
[x] Create platform library
[x] Testing with doctest
[x] Internal DynamicArray class
[x] File handling APIs
[x] Result API for error handling
[ ] Assert implementation
[ ] Create render library
[ ] Set up support for DX12 and Vulkan
[ ] Create simple app that loads GLTF, and renders in real time
[ ] Add IMGUI to window to give controls

## Infrastructure

[ ] Pull down ninja binary for building (helps Claude run from CLI)
[ ] Put cmake, python, and ninja all into the same folder?

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