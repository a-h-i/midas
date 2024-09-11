list(APPEND SOURCES_LIST main.cpp)
add_executable(midas ${SOURCES_LIST} ${PROJECT_SOURCE_DIR}/include/midas/version.h.in )

target_compile_features(midas PRIVATE cxx_std_23 )
target_compile_options(midas PRIVATE -Wall -Wextra -Wpedantic -Werror)


find_package(Boost 1.83 COMPONENTS program_options log REQUIRED)

target_include_directories(midas PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(midas PRIVATE ${Boost_LIBRARIES})

target_include_directories(midas PRIVATE ${PROJECT_BINARY_DIR}/include )

target_link_libraries(midas PRIVATE logging)
# Temp testing until we implement higher level driver bridge
target_link_libraries(midas PRIVATE ibkr_driver)

