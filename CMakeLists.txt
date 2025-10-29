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

function(add_compile_option name desc default)
    option(${name} ${desc} ${default})
    if(${name})
        target_compile_definitions(myplugin PRIVATE ${name})
    endif()
endfunction()

add_compile_option(DEBUG_NOTIFICATIONS "Show debug notifications" OFF)
add_compile_option(GROUP_TILED_WINDOWS "Create a group when a window is tiled and not in a group" OFF)

install(TARGETS myplugin)
