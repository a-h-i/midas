cmake_minimum_required(VERSION 3.28)
include(ExternalProject)
project(
        Midas
        VERSION 0.1.0
        DESCRIPTION "Algorithmic Trading"
        LANGUAGES CXX C
    )

# set(CMAKE_CXX_STANDARD 23 CACHE STRING "C++ standard to use")
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# add_compile_options(-Wall -Wextra -Wpedantic)
add_subdirectory(third-party)
add_subdirectory(apps)

configure_file (
    "include/midas/version.h.in"
    "include/midas/version.h"
)

ExternalProject_Add(inteldecimalfp
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/third-party/inteldecimalfp"
    DOWNLOAD_COMMAND ""
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_COMMAND make CC=gcc
    BUILD_IN_SOURCE 1
)



source_group(
    TREE "${PROJECT_BINARY_DIR}/include"
    PREFIX "Header Files"
    FILES ${PROJECT_BINARY_DIR}/include/midas/version.h
)

