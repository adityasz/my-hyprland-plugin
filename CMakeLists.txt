cmake_minimum_required(VERSION 3.31)

# There is no better name for a plugin that does a bunch of random stuff for me.
project(myplugin
    DESCRIPTION "My Hyprland plugin"
    VERSION 0.1
)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_FLAGS "-Wall -Wpedantic -Wextra -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "-g3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math -flto=auto")

file(GLOB_RECURSE SRC "src/*.cpp")

add_library(myplugin SHARED ${SRC})

option(DEBUG_NOTIFICATIONS "Show debug notficiations" OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET
    hyprland
    libdrm
    libinput
    libudev
    pangocairo
    pixman-1
    wayland-server
    xkbcommon
)
target_link_libraries(myplugin PRIVATE rt PkgConfig::deps)
if (DEBUG_NOTIFICATIONS)
	target_compile_definitions(myplugin PRIVATE DEBUG_NOTIFICATIONS)
endif ()

install(TARGETS myplugin)
