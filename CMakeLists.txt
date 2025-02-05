cmake_minimum_required(VERSION 3.31)
project(Renderer_engine)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define output directories for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(glib-2.0 REQUIRED)

find_package(Vulkan REQUIRED)
include_directories(include ${SDL2_INCLUDEDIR} ${Vulkan_INCLUDE_DIR} ${glib-2.0_INCLUDE_DIRS})

set(SHADER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(SHADER_OUTPUT_DIR ${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})
set(SHADERS vs ps)
foreach(SHADER ${SHADERS})
    set(SHADER_SOURCE ${SHADER_SOURCE_DIR}/${SHADER}_shader.hlsl)
    set(SHADER_OUTPUT ${SHADER_OUTPUT_DIR}/${SHADER}_shader.spv)
    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}
        COMMAND dxc -T ${SHADER}_6_0 -E main -spirv -Fo ${SHADER_OUTPUT} ${SHADER_SOURCE}
        DEPENDS ${SHADER_SOURCE}
        COMMENT "Compiling ${SHADER}_SHADER.hlsl to SPIR-V using dxc"
    )
    list(APPEND SPIRV_OUTPUTS ${SHADER_OUTPUT})
endforeach(SHADER)
add_custom_target(Shaders ALL DEPENDS ${SPIRV_OUTPUTS})

add_executable(vulkan_window
    src/sdlwindow.c
    src/core_app.c
    src/helpers.c
    src/pipelinesetup.c
    src/maininstance.c
)

target_link_libraries(vulkan_window ${SDL2_LIBRARIES} ${Vulkan_LIBRARIES})
target_compile_options(vulkan_window PRIVATE ${SDL2_CFLAGS_OTHERS})
