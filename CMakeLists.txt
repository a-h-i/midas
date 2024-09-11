cmake_minimum_required(VERSION 3.28)
include(ExternalProject)
project(
        Midas
        VERSION 0.1.0
        DESCRIPTION "Algorithmic Trading"
        LANGUAGES CXX C
    )
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

ExternalProject_Add(IntelFloatingPointDownload
    PREFIX third-party
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/third-party/inteldecimalfp"
    DOWNLOAD_COMMAND ""
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_COMMAND make CC=gcc
    BUILD_IN_SOURCE 1
)

ExternalProject_Add(
    TechnicalAnalysisDownload
    PREFIX third-party
    GIT_REPOSITORY "git@github.com:TA-Lib/ta-lib.git"
    GIT_TAG origin/main
    INSTALL_COMMAND ""
)



SET(INTEL_LIB_DIR ${CMAKE_SOURCE_DIR}/third-party/inteldecimalfp)

add_library(IntelFloatingPoint STATIC IMPORTED)
set_target_properties(IntelFloatingPoint PROPERTIES IMPORTED_LOCATION ${INTEL_LIB_DIR}/libbid.a)

SET(TA_LIB_DIR ${CMAKE_BINARY_DIR}/third-party/src/TechnicalAnalysisDownload-build/)
link_directories(${TA_LIB_DIR})

add_subdirectory(third-party)
add_subdirectory(apps)
add_subdirectory(libs)
configure_file (
    "include/midas/version.h.in"
    "include/midas/version.h"
)


source_group(
    TREE "${PROJECT_BINARY_DIR}/include"
    PREFIX "Header Files"
    FILES ${PROJECT_BINARY_DIR}/include/midas/version.h
)

