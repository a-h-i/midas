cmake_minimum_required(VERSION 3.28)
enable_testing()
include(ExternalProject)
include(FetchContent)
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set (BOOST_REQUIRED_VERSION 1.83)
add_compile_definitions(BOOST_ALL_NO_LIB)
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
    GIT_REPOSITORY "https://github.com/TA-Lib/ta-lib.git"
    GIT_TAG origin/main
    INSTALL_COMMAND ""
)
# The logging output during population can be quite verbose,
# making the configure stage quite noisy. This cache option
# (ON by default) hides all population output unless an error is encountered.
# If experiencing problems with hung downloads, temporarily
# switching this option off may help diagnose which content population
# is causing the issue.
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
    googletest
    URL  https://github.com/google/googletest/archive/0953a17a4281fc26831da647ad3fcd5e21e6473b.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v5.0.0
)
FetchContent_MakeAvailable(ftxui)


ExternalProject_Get_Property(IntelFloatingPointDownload BINARY_DIR)
SET(INTEL_BUILD_DIR ${BINARY_DIR})

add_library(IntelFloatingPoint STATIC IMPORTED)
set_target_properties(IntelFloatingPoint PROPERTIES IMPORTED_LOCATION ${INTEL_BUILD_DIR}/libbid.a)

ExternalProject_Get_Property(TechnicalAnalysisDownload BINARY_DIR)
SET(TA_BUILD_DIR ${BINARY_DIR})
ExternalProject_Get_Property(TechnicalAnalysisDownload SOURCE_DIR)
SET(TA_INCLUDE_DIR ${SOURCE_DIR}/include)
add_library(TechnicalAnalysis STATIC IMPORTED)
set_target_properties(TechnicalAnalysis PROPERTIES IMPORTED_LOCATION ${TA_BUILD_DIR}/libta_lib.a)


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

install(TARGETS midas)
