set(PROJECT_NAME Pistachio)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "src/Pistachio.h"
    "src/Pistachio/Core.h"
    "src/Pistachio/Core/Application.h"
    "src/Pistachio/Core/EntryPoint.h"
    "src/Pistachio/Core/Input.h"
    "src/Pistachio/Core/KeyCodes.h"
    "src/Pistachio/Core/Layer.h"
    "src/Pistachio/Core/LayerStack.h"
    "src/Pistachio/Core/Log.h"
    "src/Pistachio/Core/Window.h"
    "src/Pistachio/Event/ApplicationEvent.h"
    "src/Pistachio/Event/Event.h"
    "src/Pistachio/Event/KeyEvent.h"
    "src/Pistachio/Event/MouseEvent.h"
    "src/Pistachio/ImGui/ImGuiLayer.h"
    "src/Pistachio/Platform/Windows/WindowsInputCallbacks.h"
    "src/Pistachio/Platform/Windows/WindowsInputPoll.h"
    "src/Pistachio/Platform/Windows/WindowsWindow.h"
    "src/Pistachio/Renderer/Buffer.h"
    "src/Pistachio/Renderer/DirectX11/DX11Buffer.h"
    "src/Pistachio/Renderer/DirectX11/DX11Shader.h"
    "src/Pistachio/Renderer/RendererBase.h"
    "src/Pistachio/Renderer/Shader.h"
    "src/ptpch.h"
    "vendor/imgui/imconfig.h"
    "vendor/imgui/imgui.h"
    "vendor/imgui/imgui_impl_dx11.h"
    "vendor/imgui/imgui_impl_win32.h"
    "vendor/imgui/imgui_internal.h"
    "vendor/imgui/imstb_rectpack.h"
    "vendor/imgui/imstb_textedit.h"
    "vendor/imgui/imstb_truetype.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "src/Pistachio/Core/Application.cpp"
    "src/Pistachio/Core/Layer.cpp"
    "src/Pistachio/Core/LayerStack.cpp"
    "src/Pistachio/Core/Log.cpp"
    "src/Pistachio/ImGui/ImGuiLayer.cpp"
    "src/Pistachio/Platform/Windows/WindowsInputCallbacks.cpp"
    "src/Pistachio/Platform/Windows/WindowsInputPoll.cpp"
    "src/Pistachio/Platform/Windows/WindowsWindow.cpp"
    "src/Pistachio/Renderer/Buffer.cpp"
    "src/Pistachio/Renderer/DirectX11/DX11Buffer.cpp"
    "src/Pistachio/Renderer/DirectX11/DX11Shader.cpp"
    "src/Pistachio/Renderer/RendererBase.cpp"
    "src/Pistachio/Renderer/Shader.cpp"
    "src/ptpch.cpp"
    "vendor/imgui/imgui.cpp"
    "vendor/imgui/imgui_demo.cpp"
    "vendor/imgui/imgui_draw.cpp"
    "vendor/imgui/imgui_impl_dx11.cpp"
    "vendor/imgui/imgui_impl_win32.cpp"
    "vendor/imgui/imgui_tables.cpp"
    "vendor/imgui/imgui_widgets.cpp"
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
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>-${CMAKE_VS_PLATFORM_NAME}/${CMAKE_PROJECT_NAME}/"
    OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>-${CMAKE_VS_PLATFORM_NAME}/${CMAKE_PROJECT_NAME}/"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)
################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
    $<$<CONFIG:Debug>:
        MultiThreadedDebug
    >
    $<$<CONFIG:Release>:
        MultiThreaded
    >
    $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:Release>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
)
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src;"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui;"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/gainput/include"
)

################################################################################
# Compile definitions
################################################################################
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
    "PISTACHIO_RENDER_API_DX11"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /Oi;
            /Gy
        >
        /permissive-;
        /sdl;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING}
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
        /SUBSYSTEM:WINDOWS
    )
endif()

################################################################################
# Dependencies
################################################################################
set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "d3d11"
)
target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

target_link_directories(${PROJECT_NAME} PUBLIC
    "$<$<CONFIG:Debug>:"
        "${CMAKE_SOURCE_DIR}/Pistachio/vendor/gainput/lib;"
        "${CMAKE_SOURCE_DIR}/Pistachio/vendor/glfw/lib"
    ">"
)

