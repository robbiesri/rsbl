# TODO


## General 

[ ] Set up support for DX12 and Vulkan
[ ] Split code into libraries and apps
[ ] Create core, platform, and render libraries
[ ] Create simple app that loads GLTF, and renders in real time
[ ] Add IMGUI to window to give controls

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