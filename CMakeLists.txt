cmake_minimum_required(VERSION 3.25.1)
project(gltf_view LANGUAGES C CXX)

# Set policy to allow to run the target_link_libraries cmd on targets that are
# build in another directory. Currently, the linking is not handled by
# env_support/cmake/os.cmake This means that if a driver is enabled and it
# requires linking an external library it needs to be handled in the top-level
# project.
cmake_policy(SET CMP0079 NEW)

include(FetchContent)

find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

FetchContent_Declare(
  fastgltf
  GIT_REPOSITORY https://github.com/spnda/fastgltf
  GIT_TAG 4e2261350888bae7c35a1f39991f6233d57795f5
  GIT_SHALLOW TRUE)

set(FASTGLTF_ENABLE_DEPRECATED_EXT
    ON
    CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(fastgltf)

FetchContent_Declare(
  webp
  GIT_REPOSITORY https://github.com/webmproject/libwebp
  GIT_TAG fa6f56496a442eed59b103250021e4b14ebf1427
  GIT_SHALLOW TRUE)

FetchContent_MakeAvailable(webp)

# Compiler settings
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -ggdb")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -ggdb")
set(CONFIG_LV_BUILD_EXAMPLES
    OFF
    CACHE BOOL "disable lvgl examples" FORCE)
set(CONFIG_LV_BUILD_DEMOS
    OFF
    CACHE BOOL "disable lvgl demos" FORCE)
set(CONFIG_LV_USE_THORVG_INTERNAL
    OFF
    CACHE BOOL "disable thorvg internal" FORCE)

add_subdirectory(lvgl)

# Define common include directories
set(LVGL_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/lvgl ${CMAKE_SOURCE_DIR}/lvgl/src)
set(PROTO_INCLUDE_DIRS lib/lv_gltf/data/deps/)

# Create library targets
add_library(lvgl_opengl_shader_cache
            lib/lv_opengl_shader_cache/lv_opengl_shader_cache.cpp)
target_include_directories(lvgl_opengl_shader_cache
                           PRIVATE ${LVGL_INCLUDE_DIRS})

add_library(
  lvgl_gltfdata
  lib/lv_gltf/data/lv_gltf_override.cpp lib/lv_gltf/data/sup/utils.cpp
  lib/lv_gltf/data/sup/injest.cpp lib/lv_gltf/data/sup/reports.cpp
  lib/lv_gltf/data/sup/datatypes.cpp)

target_include_directories(lvgl_gltfdata PRIVATE fastgltf ${LVGL_INCLUDE_DIRS}
                                                 ${PROTO_INCLUDE_DIRS})
target_link_libraries(lvgl_gltfdata PRIVATE lvgl_opengl_shader_cache fastgltf)

target_compile_definitions(lvgl_gltfdata PRIVATE LVGL_ENABLE_WEBP_IMAGES)

add_library(
  lvgl_gltfview
  lib/lv_gltf/view/lv_gltf_view.cpp
  lib/lv_gltf/view/sup/datatypes.cpp
  lib/lv_gltf/view/sup/utils.cpp
  lib/lv_gltf/view/sup/setup.cpp
  lib/lv_gltf/view/sup/animation.cpp
  lib/lv_gltf/view/sup/ibl_sampler.cpp
  lib/lv_gltf/view/sup/shader_includes.cpp)

target_include_directories(lvgl_gltfview PRIVATE ${LVGL_INCLUDE_DIRS}
                                                 ${PROTO_INCLUDE_DIRS})
target_link_libraries(lvgl_gltfview PRIVATE lvgl_opengl_shader_cache fastgltf
                                            webp)
target_link_libraries(lvgl_gltfview PRIVATE lvgl_gltfdata fastgltf)

# Main executable
add_executable(
  gltf_view
  src/demo.cpp
  src/demo_ui.c
  src/demo_cli.c
  src/demo_nav.c
  src/demo_os_integrate.c
  src/demo_file_load_dialog.c
  src/demo_misc_utils.c
  src/mouse_cursor_icon.c
  src/lvgl_icon_40px_ARGB888.c
  src/sprites1_32x32x7.c)

add_executable(demo src/simple_demo.c)

# Build type definitions
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_definitions(-DNDEBUG)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
  add_definitions(-DNDEBUG)
endif()

# Optional: Enable AddressSanitizer for Debug builds (can be disabled if
# problematic)
option(ENABLE_ASAN "Enable AddressSanitizer in Debug builds" OFF)

if(ENABLE_ASAN AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(gltf_view PRIVATE -fsanitize=address)
  target_link_options(gltf_view PRIVATE -fsanitize=address)
endif()

# Additional optimization for MinSizeRel
if(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
  target_link_options(gltf_view PRIVATE -Wl,--gc-sections)
endif()

target_link_libraries(
  gltf_view
  PRIVATE lvgl
          lvgl_gltfview
          lvgl_gltfdata
          m
          OpenGL::GL
          GLEW::GLEW
          glfw
          webp
          fastgltf)

target_link_libraries(
  demo
  PRIVATE lvgl
          lvgl_gltfview
          lvgl_gltfdata
          m
          OpenGL::GL
          GLEW::GLEW
          glfw
          webp
          fastgltf)

target_include_directories(
  gltf_view PRIVATE ./lib/lv_gltf ./lib/lv_gltf/data/deps lvgl lvgl/src)

set_target_properties(gltf_view PROPERTIES COMPILE_DEFINITIONS
                                           "${LVGL_COMPILER_DEFINES}")

set_target_properties(demo PROPERTIES COMPILE_DEFINITIONS
                                      "${LVGL_COMPILER_DEFINES}")

target_include_directories(demo PRIVATE ./lib/lv_gltf ./lib/lv_gltf/data/deps
                                        lvgl lvgl/src)
