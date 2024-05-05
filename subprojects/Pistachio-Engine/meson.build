project('Pistachio-Engine', 'cpp',
    default_options: ['cpp_std=c++17'])
add_project_arguments(['-DPAL_STDCPP_COMPAT','-D_DEBUG'],language: 'cpp',)
cmake = import('cmake')
yaml_cpp = cmake.subproject('yaml-cpp')
yaml_dep = yaml_cpp.dependency('yaml-cpp')
rhi = subproject('RHI')
rhi_dep = rhi.get_variable('rhi_dep')



inc_dirs = ['src/',
    'vendor/entt/include/',
    'vendor/physx/include',
    'vendor/sal',
    'vendor/SimpleMath',
    'vendor/spdlog/include/',
    'vendor/stb_image/',
    'vendor/tcb_span/',
    'vendor/assimp/include/',
    'vendor/tracy/tracy',
    'vendor/DirectXMath/',
    'vendor/imgui',
    'vendor/xxhash']
if(build_machine.system() == 'linux')
    inc_dirs += '/usr/include/dxc'
endif
inc = include_directories(inc_dirs)
sources = [
    'src/Pistachio/Core/UUID.cpp',
    'src/Pistachio/Core/Layer.cpp',
    'src/Pistachio/Core/LayerStack.cpp',
    'src/Pistachio/Core/FileSystem.cpp',
    'src/Pistachio/Core/Application.cpp',
    'src/Pistachio/Core/Log.cpp',
    'src/Pistachio/Core/Transform.cpp',
    'src/Pistachio/Scene/CullingManager.cpp',
    'src/Pistachio/Scene/Scene.cpp',
    'src/Pistachio/Scene/Entity.cpp',
    'src/Pistachio/Scene/SceneSerializer.cpp',
    'src/Pistachio/Renderer/ShaderAssetCompiler.cpp',
    'src/Pistachio/Renderer/FrameComposer.cpp',
    'src/Pistachio/Renderer/RenderGraph.cpp',
    'src/Pistachio/Renderer/Renderer2D.cpp',
    'src/Pistachio/Renderer/Model.cpp',
    'src/Pistachio/Renderer/ShaderAsset.cpp',
    'src/Pistachio/Renderer/Renderer.cpp',
    'src/Pistachio/Renderer/Camera.cpp',
    'src/Pistachio/Renderer/EditorCamera.cpp',
    'src/Pistachio/Renderer/MeshFactory.cpp',
    'src/Pistachio/Renderer/RenderTexture.cpp',
    'src/Pistachio/Renderer/RendererBase.cpp',
    'src/Pistachio/Renderer/Mesh.cpp',
    'src/Pistachio/Renderer/Material.cpp',
    'src/Pistachio/Asset/AssetManager.cpp',
    'src/Pistachio/Physics/Physics.cpp',
    'src/Pistachio/Allocators/FreeList.cpp',
    'src/Pistachio/Allocators/AtlasAllocator.cpp',
]
lib = shared_library('Pistachio',sources,include_directories:inc,dependencies:[rhi_dep,yaml_dep])
pistachio_dep = declare_dependency(include_directories:inc, link_with:lib)