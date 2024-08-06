project('Pistachio-Engine', 'cpp',
    default_options: ['cpp_std=c++20'])
cmake = import('cmake')

dxc = subproject('dxc')
dxc_dep = dxc.get_variable('dxc_dep')

yaml_cpp = cmake.subproject('yaml-cpp')
yaml_dep = yaml_cpp.dependency('yaml-cpp')

assimp = cmake.subproject('assimp')
assimp_dep = assimp.dependency('assimp')

glfw_opt = cmake.subproject_options()
glfw_opt.add_cmake_defines({'GLFW_BUILD_WAYLAND':get_option('linux_compositor') == 'wayland'})
glfw_opt.add_cmake_defines({'GLFW_BUILD_X11':get_option('linux_compositor') == 'x11'})
glfw3 = cmake.subproject('glfw',options: glfw_opt)
glfw3_dep = glfw3.dependency('glfw')

angelscript = subproject('angelscript')
angelscript_dep = angelscript.get_variable('angelscript_dep')
as_addons_dep = angelscript.get_variable('as_addons_dep')

rhi = subproject('RHI')
rhi_dep = rhi.get_variable('rhi_dep')

imgui = subproject('imgui')
imgui_dep = imgui.get_variable('imgui_dep')

spdlog_opt = cmake.subproject_options()
spdlog_opt.add_cmake_defines({'SPDLOG_BUILD_PIC':true})
spdlog = cmake.subproject('spdlog',options:spdlog_opt)
spdlog_dep = spdlog.dependency('spdlog')

xxhash = subproject('xxhash')
xxhash_dep = xxhash.get_variable('xxhash_dep')

physx = subproject('physx')
px_deps = physx.get_variable('px_all_deps')

# Normally we would depend on ezr but RHI provides it
# ezr = subproject('ezr')
# ezr_dep = ezr.get_variable('ezr_dep')
deps = [
    rhi_dep,
    yaml_dep,
    assimp_dep,
    imgui_dep,
    spdlog_dep,
    xxhash_dep,
    px_deps,
    angelscript_dep,
    as_addons_dep,
    dxc_dep,]

inc_dirs = ['src/',
    'vendor/entt/include/',
    'vendor/sal',
    'vendor/SimpleMath',
    'vendor/stb_image/',
    'vendor/tcb_span/',
    'vendor/tracy/tracy',
    'vendor/DirectXMath/',]

sources = [
    'src/Pistachio/Core/UUID.cpp',
    'src/Pistachio/Core/Layer.cpp',
    'src/Pistachio/Core/LayerStack.cpp',
    'src/Pistachio/Core/FileSystem.cpp',
    'src/Pistachio/Core/Application.cpp',
    'src/Pistachio/Core/Log.cpp',
    'src/Pistachio/Core/Input.cpp',
    'src/Pistachio/Core/Transform.cpp',
    'src/Pistachio/Core/InputCallbacks.cpp',
    'src/Pistachio/Scene/CullingManager.cpp',
    'src/Pistachio/Scene/Scene.cpp',
    'src/Pistachio/Scene/Entity.cpp',
    'src/Pistachio/Scene/SceneSerializer.cpp',
    'src/Pistachio/Renderer/ShaderAssetCompiler.cpp',
    'src/Pistachio/Renderer/FrameComposer.cpp',
    'src/Pistachio/Renderer/RenderGraph.cpp',
    'src/Pistachio/Renderer/Renderer2D.cpp',
    'src/Pistachio/Renderer/RendererContext.cpp',
    'src/Pistachio/Renderer/Model.cpp',
    'src/Pistachio/Renderer/ShaderAsset.cpp',
    'src/Pistachio/Renderer/Renderer.cpp',
    'src/Pistachio/Renderer/Camera.cpp',
    'src/Pistachio/Renderer/Shader.cpp',
    'src/Pistachio/Renderer/Buffer.cpp',
    'src/Pistachio/Renderer/Texture.cpp',
    'src/Pistachio/Renderer/EditorCamera.cpp',
    'src/Pistachio/Renderer/MeshFactory.cpp',
    'src/Pistachio/Renderer/RenderTexture.cpp',
    'src/Pistachio/Renderer/RendererBase.cpp',
    'src/Pistachio/Renderer/Mesh.cpp',
    'src/Pistachio/Renderer/Material.cpp',
    'src/Pistachio/Asset/AssetManager.cpp',
    'src/Pistachio/Physics/Physics.cpp',
    'src/Pistachio/Allocators/FreeList.cpp',
    'src/Pistachio/Scripting/AngelScript/script_file.cpp',
    'src/Pistachio/Scripting/AngelScript/ScriptAPIBase.cpp',
    'src/Pistachio/Scripting/AngelScript/ScriptAPI_ECS.cpp',
    'src/Pistachio/Allocators/AtlasAllocator.cpp',
    'vendor/SimpleMath/SimpleMath.cpp', #we don't need the entire DXTK
    'vendor/stb_image/stb_image.cpp'
]
tests_src = [
    'tests/script_test.cpp'
]
args = ['-DPAL_STDCPP_COMPAT','-D_DEBUG']
if(build_machine.system() == 'linux')
    deps += glfw3_dep
    args += '-DUSE_GLFW'
    args += '-DPT_PLATFORM_LINUX'
    sources += 'src/Pistachio/Platform/Linux/LinuxWindow.cpp'
    sources += 'src/Pistachio/Platform/Linux/LinuxInputPoll.cpp'
endif
add_project_arguments(args,language: 'cpp',)
inc = include_directories(inc_dirs)
lib = shared_library('Pistachio',sources,include_directories:inc,dependencies:deps,
    override_options: ['cpp_std=c++20'])
pistachio_dep = [
    declare_dependency(include_directories:inc, link_with:lib), deps
]
exe = executable('Pistachio-Tests', tests_src,dependencies: pistachio_dep)