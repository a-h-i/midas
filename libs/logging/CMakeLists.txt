file(GLOB HEADER_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/logging/*.hpp")


add_library(logging logging.cpp ${HEADER_FILES})

target_compile_features(logging PUBLIC cxx_std_23)
target_compile_options(logging PRIVATE -Wall -Wextra -Wpedantic -Werror)

# Public interface headers
target_include_directories(logging PUBLIC ../../include)


find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED COMPONENTS log filesystem log_setup date_time REQUIRED)
target_include_directories(logging PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(logging PUBLIC ${Boost_LIBRARIES})