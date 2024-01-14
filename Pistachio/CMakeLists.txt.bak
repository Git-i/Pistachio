set(PROJECT_NAME Pistachio)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "src/Pistachio.h"
    "src/Pistachio/Core.h"
    "src/Pistachio/Core/Application.h"
    "src/Pistachio/Core/EntryPoint.h"
    "src/Pistachio/Core/Error.h"
    "src/Pistachio/Core/Input.h"
    "src/Pistachio/Core/KeyCodes.h"
    "src/Pistachio/Core/Layer.h"
    "src/Pistachio/Core/LayerStack.h"
    "src/Pistachio/Core/Log.h"
    "src/Pistachio/Core/Timestep.h"
    "src/Pistachio/Core/Transform.h"
    "src/Pistachio/Core/Window.h"
    "src/Pistachio/Debug/Instrumentor.h"
    "src/Pistachio/Event/ApplicationEvent.h"
    "src/Pistachio/Event/Event.h"
    "src/Pistachio/Event/KeyEvent.h"
    "src/Pistachio/Event/MouseEvent.h"
    "src/Pistachio/ImGui/ImGuiLayer.h"
    "src/Pistachio/Platform/Windows/WindowsInputCallbacks.h"
    "src/Pistachio/Platform/Windows/WindowsInputPoll.h"
    "src/Pistachio/Platform/Windows/WindowsWindow.h"
    "src/Pistachio/Renderer/Buffer.h"
    "src/Pistachio/Renderer/Camera.h"
    "src/Pistachio/Renderer/DirectX11/DX11Buffer.h"
    "src/Pistachio/Renderer/DirectX11/DX11Cubemap.h"
    "src/Pistachio/Renderer/DirectX11/DX11RendererBase.h"
    "src/Pistachio/Renderer/DirectX11/DX11RenderTexture.h"
    "src/Pistachio/Renderer/DirectX11/DX11Shader.h"
    "src/Pistachio/Renderer/DirectX11/DX11Texture.h"
    "src/Pistachio/Renderer/Mesh.h"
    "src/Pistachio/Renderer/Model.h"
    "src/Pistachio/Renderer/Renderer.h"
    "src/Pistachio/Renderer/Renderer2D.h"
    "src/Pistachio/Renderer/RendererBase.h"
    "src/Pistachio/Renderer/RenderTexture.h"
    "src/Pistachio/Renderer/Sampler.h"
    "src/Pistachio/Renderer/Shader.h"
    "src/Pistachio/Renderer/Texture.h"
    "src/Pistachio/Scene/Components.h"
    "src/Pistachio/Scene/Entity.h"
    "src/Pistachio/Scene/Scene.h"
    "src/ptpch.h"
    "vendor/assimp/include/assimp/aabb.h"
    "vendor/assimp/include/assimp/ai_assert.h"
    "vendor/assimp/include/assimp/anim.h"
    "vendor/assimp/include/assimp/Base64.hpp"
    "vendor/assimp/include/assimp/BaseImporter.h"
    "vendor/assimp/include/assimp/Bitmap.h"
    "vendor/assimp/include/assimp/BlobIOSystem.h"
    "vendor/assimp/include/assimp/ByteSwapper.h"
    "vendor/assimp/include/assimp/camera.h"
    "vendor/assimp/include/assimp/cexport.h"
    "vendor/assimp/include/assimp/cfileio.h"
    "vendor/assimp/include/assimp/cimport.h"
    "vendor/assimp/include/assimp/ColladaMetaData.h"
    "vendor/assimp/include/assimp/color4.h"
    "vendor/assimp/include/assimp/color4.inl"
    "vendor/assimp/include/assimp/commonMetaData.h"
    "vendor/assimp/include/assimp/Compiler/poppack1.h"
    "vendor/assimp/include/assimp/Compiler/pstdint.h"
    "vendor/assimp/include/assimp/Compiler/pushpack1.h"
    "vendor/assimp/include/assimp/config.h"
    "vendor/assimp/include/assimp/CreateAnimMesh.h"
    "vendor/assimp/include/assimp/DefaultIOStream.h"
    "vendor/assimp/include/assimp/DefaultIOSystem.h"
    "vendor/assimp/include/assimp/DefaultLogger.hpp"
    "vendor/assimp/include/assimp/defs.h"
    "vendor/assimp/include/assimp/Exceptional.h"
    "vendor/assimp/include/assimp/Exporter.hpp"
    "vendor/assimp/include/assimp/fast_atof.h"
    "vendor/assimp/include/assimp/GenericProperty.h"
    "vendor/assimp/include/assimp/GltfMaterial.h"
    "vendor/assimp/include/assimp/Hash.h"
    "vendor/assimp/include/assimp/Importer.hpp"
    "vendor/assimp/include/assimp/importerdesc.h"
    "vendor/assimp/include/assimp/IOStream.hpp"
    "vendor/assimp/include/assimp/IOStreamBuffer.h"
    "vendor/assimp/include/assimp/IOSystem.hpp"
    "vendor/assimp/include/assimp/light.h"
    "vendor/assimp/include/assimp/LineSplitter.h"
    "vendor/assimp/include/assimp/LogAux.h"
    "vendor/assimp/include/assimp/Logger.hpp"
    "vendor/assimp/include/assimp/LogStream.hpp"
    "vendor/assimp/include/assimp/material.h"
    "vendor/assimp/include/assimp/material.inl"
    "vendor/assimp/include/assimp/MathFunctions.h"
    "vendor/assimp/include/assimp/matrix3x3.h"
    "vendor/assimp/include/assimp/matrix3x3.inl"
    "vendor/assimp/include/assimp/matrix4x4.h"
    "vendor/assimp/include/assimp/matrix4x4.inl"
    "vendor/assimp/include/assimp/MemoryIOWrapper.h"
    "vendor/assimp/include/assimp/mesh.h"
    "vendor/assimp/include/assimp/metadata.h"
    "vendor/assimp/include/assimp/NullLogger.hpp"
    "vendor/assimp/include/assimp/ObjMaterial.h"
    "vendor/assimp/include/assimp/ParsingUtils.h"
    "vendor/assimp/include/assimp/pbrmaterial.h"
    "vendor/assimp/include/assimp/port/AndroidJNI/AndroidJNIIOSystem.h"
    "vendor/assimp/include/assimp/port/AndroidJNI/BundledAssetIOSystem.h"
    "vendor/assimp/include/assimp/postprocess.h"
    "vendor/assimp/include/assimp/Profiler.h"
    "vendor/assimp/include/assimp/ProgressHandler.hpp"
    "vendor/assimp/include/assimp/qnan.h"
    "vendor/assimp/include/assimp/quaternion.h"
    "vendor/assimp/include/assimp/quaternion.inl"
    "vendor/assimp/include/assimp/RemoveComments.h"
    "vendor/assimp/include/assimp/scene.h"
    "vendor/assimp/include/assimp/SceneCombiner.h"
    "vendor/assimp/include/assimp/SGSpatialSort.h"
    "vendor/assimp/include/assimp/SkeletonMeshBuilder.h"
    "vendor/assimp/include/assimp/SmallVector.h"
    "vendor/assimp/include/assimp/SmoothingGroups.h"
    "vendor/assimp/include/assimp/SmoothingGroups.inl"
    "vendor/assimp/include/assimp/SpatialSort.h"
    "vendor/assimp/include/assimp/StandardShapes.h"
    "vendor/assimp/include/assimp/StreamReader.h"
    "vendor/assimp/include/assimp/StreamWriter.h"
    "vendor/assimp/include/assimp/StringComparison.h"
    "vendor/assimp/include/assimp/StringUtils.h"
    "vendor/assimp/include/assimp/Subdivision.h"
    "vendor/assimp/include/assimp/texture.h"
    "vendor/assimp/include/assimp/TinyFormatter.h"
    "vendor/assimp/include/assimp/types.h"
    "vendor/assimp/include/assimp/vector2.h"
    "vendor/assimp/include/assimp/vector2.inl"
    "vendor/assimp/include/assimp/vector3.h"
    "vendor/assimp/include/assimp/vector3.inl"
    "vendor/assimp/include/assimp/version.h"
    "vendor/assimp/include/assimp/Vertex.h"
    "vendor/assimp/include/assimp/XmlParser.h"
    "vendor/assimp/include/assimp/XMLTools.h"
    "vendor/assimp/include/assimp/ZipArchiveIOSystem.h"
    "vendor/entt/include/entt.hpp"
    "vendor/gainput/include/gainput/gainput.h"
    "vendor/gainput/include/gainput/GainputAllocator.h"
    "vendor/gainput/include/gainput/GainputContainers.h"
    "vendor/gainput/include/gainput/GainputDebugRenderer.h"
    "vendor/gainput/include/gainput/GainputHelpers.h"
    "vendor/gainput/include/gainput/GainputInputDeltaState.h"
    "vendor/gainput/include/gainput/GainputInputDevice.h"
    "vendor/gainput/include/gainput/GainputInputDeviceBuiltIn.h"
    "vendor/gainput/include/gainput/GainputInputDeviceKeyboard.h"
    "vendor/gainput/include/gainput/GainputInputDeviceMouse.h"
    "vendor/gainput/include/gainput/GainputInputDevicePad.h"
    "vendor/gainput/include/gainput/GainputInputDeviceTouch.h"
    "vendor/gainput/include/gainput/GainputInputListener.h"
    "vendor/gainput/include/gainput/GainputInputManager.h"
    "vendor/gainput/include/gainput/GainputInputMap.h"
    "vendor/gainput/include/gainput/GainputInputState.h"
    "vendor/gainput/include/gainput/GainputIos.h"
    "vendor/gainput/include/gainput/GainputLog.h"
    "vendor/gainput/include/gainput/GainputMapFilters.h"
    "vendor/gainput/include/gainput/gestures/GainputButtonStickGesture.h"
    "vendor/gainput/include/gainput/gestures/GainputDoubleClickGesture.h"
    "vendor/gainput/include/gainput/gestures/GainputGestures.h"
    "vendor/gainput/include/gainput/gestures/GainputHoldGesture.h"
    "vendor/gainput/include/gainput/gestures/GainputPinchGesture.h"
    "vendor/gainput/include/gainput/gestures/GainputRotateGesture.h"
    "vendor/gainput/include/gainput/gestures/GainputSimultaneouslyDownGesture.h"
    "vendor/gainput/include/gainput/gestures/GainputTapGesture.h"
    "vendor/gainput/include/gainput/recorder/GainputInputPlayer.h"
    "vendor/gainput/include/gainput/recorder/GainputInputRecorder.h"
    "vendor/gainput/include/gainput/recorder/GainputInputRecording.h"
    "vendor/glfw/include/GLFW/glfw3.h"
    "vendor/glfw/include/GLFW/glfw3native.h"
    "vendor/imgui/imconfig.h"
    "vendor/imgui/imgui.h"
    "vendor/imgui/imgui_impl_dx11.h"
    "vendor/imgui/imgui_impl_win32.h"
    "vendor/imgui/imgui_internal.h"
    "vendor/imgui/imstb_rectpack.h"
    "vendor/imgui/imstb_textedit.h"
    "vendor/imgui/imstb_truetype.h"
    "vendor/spdlog/include/spdlog/async.h"
    "vendor/spdlog/include/spdlog/async_logger-inl.h"
    "vendor/spdlog/include/spdlog/async_logger.h"
    "vendor/spdlog/include/spdlog/cfg/argv.h"
    "vendor/spdlog/include/spdlog/cfg/env.h"
    "vendor/spdlog/include/spdlog/cfg/helpers-inl.h"
    "vendor/spdlog/include/spdlog/cfg/helpers.h"
    "vendor/spdlog/include/spdlog/common-inl.h"
    "vendor/spdlog/include/spdlog/common.h"
    "vendor/spdlog/include/spdlog/details/backtracer-inl.h"
    "vendor/spdlog/include/spdlog/details/backtracer.h"
    "vendor/spdlog/include/spdlog/details/circular_q.h"
    "vendor/spdlog/include/spdlog/details/console_globals.h"
    "vendor/spdlog/include/spdlog/details/file_helper-inl.h"
    "vendor/spdlog/include/spdlog/details/file_helper.h"
    "vendor/spdlog/include/spdlog/details/fmt_helper.h"
    "vendor/spdlog/include/spdlog/details/log_msg-inl.h"
    "vendor/spdlog/include/spdlog/details/log_msg.h"
    "vendor/spdlog/include/spdlog/details/log_msg_buffer-inl.h"
    "vendor/spdlog/include/spdlog/details/log_msg_buffer.h"
    "vendor/spdlog/include/spdlog/details/mpmc_blocking_q.h"
    "vendor/spdlog/include/spdlog/details/null_mutex.h"
    "vendor/spdlog/include/spdlog/details/os-inl.h"
    "vendor/spdlog/include/spdlog/details/os.h"
    "vendor/spdlog/include/spdlog/details/periodic_worker-inl.h"
    "vendor/spdlog/include/spdlog/details/periodic_worker.h"
    "vendor/spdlog/include/spdlog/details/registry-inl.h"
    "vendor/spdlog/include/spdlog/details/registry.h"
    "vendor/spdlog/include/spdlog/details/synchronous_factory.h"
    "vendor/spdlog/include/spdlog/details/tcp_client-windows.h"
    "vendor/spdlog/include/spdlog/details/tcp_client.h"
    "vendor/spdlog/include/spdlog/details/thread_pool-inl.h"
    "vendor/spdlog/include/spdlog/details/thread_pool.h"
    "vendor/spdlog/include/spdlog/details/udp_client-windows.h"
    "vendor/spdlog/include/spdlog/details/udp_client.h"
    "vendor/spdlog/include/spdlog/details/windows_include.h"
    "vendor/spdlog/include/spdlog/fmt/bin_to_hex.h"
    "vendor/spdlog/include/spdlog/fmt/bundled/args.h"
    "vendor/spdlog/include/spdlog/fmt/bundled/chrono.h"
    "vendor/spdlog/include/spdlog/fmt/bundled/color.h"
    "vendor/spdlog/include/spdlog/fmt/bundled/compile.h"
    "vendor/spdlog/include/spdlog/fmt/bundled/core.h"
    "vendor/spdlog/include/spdlog/fmt/bundled/format-inl.h"
    "vendor/spdlog/include/spdlog/fmt/bundled/format.h"
    "vendor/spdlog/include/spdlog/fmt/bundled/locale.h"
    "vendor/spdlog/include/spdlog/fmt/bundled/os.h"
    "vendor/spdlog/include/spdlog/fmt/bundled/ostream.h"
    "vendor/spdlog/include/spdlog/fmt/bundled/printf.h"
    "vendor/spdlog/include/spdlog/fmt/bundled/ranges.h"
    "vendor/spdlog/include/spdlog/fmt/bundled/std.h"
    "vendor/spdlog/include/spdlog/fmt/bundled/xchar.h"
    "vendor/spdlog/include/spdlog/fmt/chrono.h"
    "vendor/spdlog/include/spdlog/fmt/compile.h"
    "vendor/spdlog/include/spdlog/fmt/fmt.h"
    "vendor/spdlog/include/spdlog/fmt/ostr.h"
    "vendor/spdlog/include/spdlog/fmt/ranges.h"
    "vendor/spdlog/include/spdlog/fmt/std.h"
    "vendor/spdlog/include/spdlog/fmt/xchar.h"
    "vendor/spdlog/include/spdlog/formatter.h"
    "vendor/spdlog/include/spdlog/fwd.h"
    "vendor/spdlog/include/spdlog/logger-inl.h"
    "vendor/spdlog/include/spdlog/logger.h"
    "vendor/spdlog/include/spdlog/pattern_formatter-inl.h"
    "vendor/spdlog/include/spdlog/pattern_formatter.h"
    "vendor/spdlog/include/spdlog/sinks/android_sink.h"
    "vendor/spdlog/include/spdlog/sinks/ansicolor_sink-inl.h"
    "vendor/spdlog/include/spdlog/sinks/ansicolor_sink.h"
    "vendor/spdlog/include/spdlog/sinks/base_sink-inl.h"
    "vendor/spdlog/include/spdlog/sinks/base_sink.h"
    "vendor/spdlog/include/spdlog/sinks/basic_file_sink-inl.h"
    "vendor/spdlog/include/spdlog/sinks/basic_file_sink.h"
    "vendor/spdlog/include/spdlog/sinks/daily_file_sink.h"
    "vendor/spdlog/include/spdlog/sinks/dist_sink.h"
    "vendor/spdlog/include/spdlog/sinks/dup_filter_sink.h"
    "vendor/spdlog/include/spdlog/sinks/hourly_file_sink.h"
    "vendor/spdlog/include/spdlog/sinks/mongo_sink.h"
    "vendor/spdlog/include/spdlog/sinks/msvc_sink.h"
    "vendor/spdlog/include/spdlog/sinks/null_sink.h"
    "vendor/spdlog/include/spdlog/sinks/ostream_sink.h"
    "vendor/spdlog/include/spdlog/sinks/qt_sinks.h"
    "vendor/spdlog/include/spdlog/sinks/ringbuffer_sink.h"
    "vendor/spdlog/include/spdlog/sinks/rotating_file_sink-inl.h"
    "vendor/spdlog/include/spdlog/sinks/rotating_file_sink.h"
    "vendor/spdlog/include/spdlog/sinks/sink-inl.h"
    "vendor/spdlog/include/spdlog/sinks/sink.h"
    "vendor/spdlog/include/spdlog/sinks/stdout_color_sinks-inl.h"
    "vendor/spdlog/include/spdlog/sinks/stdout_color_sinks.h"
    "vendor/spdlog/include/spdlog/sinks/stdout_sinks-inl.h"
    "vendor/spdlog/include/spdlog/sinks/stdout_sinks.h"
    "vendor/spdlog/include/spdlog/sinks/syslog_sink.h"
    "vendor/spdlog/include/spdlog/sinks/systemd_sink.h"
    "vendor/spdlog/include/spdlog/sinks/tcp_sink.h"
    "vendor/spdlog/include/spdlog/sinks/udp_sink.h"
    "vendor/spdlog/include/spdlog/sinks/win_eventlog_sink.h"
    "vendor/spdlog/include/spdlog/sinks/wincolor_sink-inl.h"
    "vendor/spdlog/include/spdlog/sinks/wincolor_sink.h"
    "vendor/spdlog/include/spdlog/spdlog-inl.h"
    "vendor/spdlog/include/spdlog/spdlog.h"
    "vendor/spdlog/include/spdlog/stopwatch.h"
    "vendor/spdlog/include/spdlog/tweakme.h"
    "vendor/spdlog/include/spdlog/version.h"
    "vendor/stb_image/stb_image.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "src/Pistachio/Core/Application.cpp"
    "src/Pistachio/Core/Layer.cpp"
    "src/Pistachio/Core/LayerStack.cpp"
    "src/Pistachio/Core/Log.cpp"
    "src/Pistachio/Core/Transform.cpp"
    "src/Pistachio/ImGui/ImGuiLayer.cpp"
    "src/Pistachio/Platform/Windows/WindowsInputCallbacks.cpp"
    "src/Pistachio/Platform/Windows/WindowsInputPoll.cpp"
    "src/Pistachio/Platform/Windows/WindowsWindow.cpp"
    "src/Pistachio/Renderer/Buffer.cpp"
    "src/Pistachio/Renderer/Camera.cpp"
    "src/Pistachio/Renderer/DirectX11/DX11Buffer.cpp"
    "src/Pistachio/Renderer/DirectX11/DX11Cubemap.cpp"
    "src/Pistachio/Renderer/DirectX11/DX11RendererBase.cpp"
    "src/Pistachio/Renderer/DirectX11/DX11RenderTexture.cpp"
    "src/Pistachio/Renderer/DirectX11/DX11Shader.cpp"
    "src/Pistachio/Renderer/DirectX11/DX11Texture.cpp"
    "src/Pistachio/Renderer/Mesh.cpp"
    "src/Pistachio/Renderer/Model.cpp"
    "src/Pistachio/Renderer/Renderer.cpp"
    "src/Pistachio/Renderer/Renderer2D.cpp"
    "src/Pistachio/Renderer/RendererBase.cpp"
    "src/Pistachio/Renderer/RenderTexture.cpp"
    "src/Pistachio/Renderer/Sampler.cpp"
    "src/Pistachio/Renderer/Shader.cpp"
    "src/Pistachio/Renderer/Texture.cpp"
    "src/Pistachio/Scene/Entity.cpp"
    "src/Pistachio/Scene/Scene.cpp"
    "src/ptpch.cpp"
    "vendor/imgui/imgui.cpp"
    "vendor/imgui/imgui_demo.cpp"
    "vendor/imgui/imgui_draw.cpp"
    "vendor/imgui/imgui_impl_dx11.cpp"
    "vendor/imgui/imgui_impl_win32.cpp"
    "vendor/imgui/imgui_tables.cpp"
    "vendor/imgui/imgui_widgets.cpp"
    "vendor/stb_image/stb_image.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

target_precompile_headers(${PROJECT_NAME} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/ptpch.h>"
)

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE Tempest)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
################################################################################
# Output directory
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>-${CMAKE_VS_PLATFORM_NAME}/${CMAKE_PROJECT_NAME}/"
        OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>-${CMAKE_VS_PLATFORM_NAME}/${CMAKE_PROJECT_NAME}/"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>-${CMAKE_VS_PLATFORM_NAME}/${CMAKE_PROJECT_NAME}/"
        OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>-${CMAKE_VS_PLATFORM_NAME}/${CMAKE_PROJECT_NAME}/"
    )
endif()
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
endif()
################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:Debug>:
            MultiThreadedDebugDLL
        >
        $<$<CONFIG:Release>:
            MultiThreadedDLL
        >
        $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:Release>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:Debug>:
            MultiThreadedDebugDLL
        >
        $<$<CONFIG:Release>:
            MultiThreadedDLL
        >
        $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:Release>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Include directories
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "$<$<CONFIG:Debug>:"
            "${CMAKE_CURRENT_SOURCE_DIR}/vendor/entt/include"
        ">"
        "${CMAKE_CURRENT_SOURCE_DIR}/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui;"
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image;"
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/assimp/include"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "$<$<CONFIG:Debug>:"
            "${CMAKE_CURRENT_SOURCE_DIR}/vendor/entt/include"
        ">"
        "${CMAKE_CURRENT_SOURCE_DIR}/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui;"
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image;"
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/assimp/include"
    )
endif()

################################################################################
# Compile definitions
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG;"
            "PISTACHIO_BUILD_DLL"
        ">"
        "_CONSOLE;"
        "PT_PLATFORM_WINDOWS;"
        "PISTACHIO_RENDER_API_DX11;"
        "IMGUI"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG;"
            "PISTACHIO_BUILD_DLL"
        ">"
        "_CONSOLE;"
        "PT_PLATFORM_WINDOWS;"
        "PISTACHIO_RENDER_API_DX11;"
        "IMGUI"
    )
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy;
                /FAs
            >
            /permissive-;
            /std:c++17;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy;
                /FAs
            >
            /permissive-;
            /std:c++17;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF
            >
            /SUBSYSTEM:WINDOWS
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF
            >
            /SUBSYSTEM:WINDOWS
        )
    endif()
endif()

################################################################################
# Dependencies
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:Debug>:"
            "assimp"
        ">"
        "$<$<CONFIG:Release>:"
            "d3d11;"
            "assimp-vc143-mt;"
            "zlibstatic"
        ">"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:Debug>:"
            "assimp"
        ">"
        "$<$<CONFIG:Release>:"
            "d3d11;"
            "assimp-vc143-mt;"
            "zlibstatic"
        ">"
    )
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_link_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_SOURCE_DIR}/Pistachio/vendor/assimp"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_link_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_SOURCE_DIR}/Pistachio/vendor/assimp"
    )
endif()

