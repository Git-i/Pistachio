# Pistachio
Pistachio Game Engine is a C++ 20 game engine utilizing vulkan and d3d12 (currently broken).

## Building
### Requirements
- Vulkan SDK
- Meson
- Ninja or Visual Studio
- (Meson will handle the rest)
### Steps
Clone the repository:
```
git clone https://github.com/Git-i/Pistachio.git
```
**Notes**: 
- Even though it has submodules, no need for a recursive clone as Meson would handle that
- You could use `--depth 1` to reduce size

Setup the builddir
```
cd Pistachio
meson setup builddir
```
Build with meson
```
cd builddir
meson compile
```



