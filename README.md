# rsbl

Yet another real-time rendering framework

## Non-Hermetic Dependencies

I've strived to make sure the build here has all the components needed to support building and running this project.

However, the current infrastructure does have some user setup of external dependencies.

* Visual Studio 2022 install on Windows
    * Tested against VS 17.12, VC 1941, Build Tools 14.41
* Windows 11 SDK - tested against 22H2 (10.0.22621.0)
* [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) - tested against 1.4.335.0