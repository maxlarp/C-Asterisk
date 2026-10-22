cmake_minimum_required(VERSION 3.15)

project(Encode VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# minifb picks the X11 backend on Linux unless this option is already in the
# cache. Seed it before add_subdirectory() so the default build talks Wayland.
# Override with -DMINIFB_USE_WAYLAND_API=OFF to go back to X11.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT DEFINED MINIFB_USE_WAYLAND_API)
    set(MINIFB_USE_WAYLAND_API ON CACHE BOOL "Build minifb against Wayland instead of X11")
endif()

add_executable(main main.c)

add_subdirectory(dependencies/minifb)

target_link_libraries(main minifb)
