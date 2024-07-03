# Pistachio
Pistachio Game Engine is a C++ 17 game engine utilizing vulkan and d3d12 (currently broken).

## Building
### Requirements
- Physx-5
- Vulkan SDK
- dxc (probably preinstalled with the windows sdk)
### Steps ?
The engine can be built with meson, some commits may not build, so just wait for a newer one or try an older one that builds. The build process involves manually building physx5 and placing the generated libraries under `subprojects/physx` or installing them system wide, and doing the same with dxc. Then proceeding with a normal meson build, note that first build will probably require an internet connection and some submodules require ssh to github (I'm working on it).


