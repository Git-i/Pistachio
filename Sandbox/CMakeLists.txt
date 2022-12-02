set(PROJECT_NAME Sandbox)

################################################################################
# Source groups
################################################################################
set(Source_Files
    "src/SandBoxApp.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE Sandbox)

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
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "$<$<CONFIG:Debug>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Pistachio/vendor/glfw/include"
    ">"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Pistachio/src;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Pistachio/vendor/spdlog/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Pistachio/vendor/gainput/include"
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "_DEBUG;"
        ""
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG"
    ">"
    "_CONSOLE;"
    "%(PreprocessorDefinitions)PT_PLATFORM_WINDOWS;"
    "UNICODE;"
    "_UNICODE"
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
    string(CONCAT FILE_CL_OPTIONS
        "$<$<CONFIG:Debug>:"
            "/Y-"
        ">"
    )
    source_file_compile_options(src/SandBoxApp.cpp ${FILE_CL_OPTIONS})
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
        /DEBUG;
        /SUBSYSTEM:WINDOWS
    )
endif()

################################################################################
# Post build events
################################################################################
add_custom_command_if(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMANDS
    COMMAND   $<CONFIG:Debug> copy $<SHELL_PATH:${CMAKE_SOURCE_DIR}/>bin\\$<CONFIG>-${CMAKE_VS_PLATFORM_NAME}\\Pistachio\\Pistachio.dll $<SHELL_PATH:${CMAKE_SOURCE_DIR}/>bin\\$<CONFIG>-${CMAKE_VS_PLATFORM_NAME}\\${PROJECT_NAME}
    COMMAND $<CONFIG:Release> copy $<SHELL_PATH:${CMAKE_SOURCE_DIR}/>bin\\$<CONFIG>-${CMAKE_VS_PLATFORM_NAME}\\Pistachio\\Pistachio.dll $<SHELL_PATH:${CMAKE_SOURCE_DIR}/>bin\\$<CONFIG>-${CMAKE_VS_PLATFORM_NAME}\\${PROJECT_NAME}
)

################################################################################
# Dependencies
################################################################################
# Link with other targets.
target_link_libraries(${PROJECT_NAME} PRIVATE
    Pistachio
)

set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "$(CoreLibraryDependencies)"
)
target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

