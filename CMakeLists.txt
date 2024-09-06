cmake_minimum_required(VERSION 3.28)

project(
        Midas
        VERSION 0.1
        DESCRIPTION "Algorithmic Trading"
        LANGUAGES CXX
    )

set(CMAKE_CXX_STANDARD 23 CACHE STRING "C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_compile_options(-Wall -Wextra -Wpedantic)

