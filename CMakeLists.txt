cmake_minimum_required(VERSION 3.31)

project(focusorlaunch
    DESCRIPTION "FocusOrLaunch plugin for Hyprland"
    VERSION 0.1
)

set(CMAKE_CXX_STANDARD 26)

file(GLOB_RECURSE SRC "src/*.cpp")

add_library(focusorlaunch SHARED ${SRC})

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
target_link_libraries(focusorlaunch PRIVATE rt PkgConfig::deps)

install(TARGETS focusorlaunch)
