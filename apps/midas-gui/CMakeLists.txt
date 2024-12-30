

set(SOURCES_LIST main.cpp)

add_executable(midas-gui ${SOURCES_LIST})
target_compile_features(midas-gui PRIVATE cxx_std_23)
target_compile_options(midas-gui PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(midas-gui PRIVATE gui)