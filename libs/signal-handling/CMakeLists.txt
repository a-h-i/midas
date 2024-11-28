
file(GLOB HEADER_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/signal-handling/*.hpp")

set(SOURCE_LIST handler.cpp)


add_library(signal_handling ${SOURCE_LIST} ${HEADER_FILES})

target_compile_features(signal_handling PUBLIC cxx_std_23)
target_compile_options(signal_handling PRIVATE -Wall -Wextra -Wpedantic)


# Public interface headers
target_include_directories(signal_handling PUBLIC ../../include)

# Thread support
find_package(Threads REQUIRED)
target_link_libraries(signal_handling PUBLIC Threads::Threads)


# Boost
find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED)
target_include_directories(signal_handling PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(signal_handling PUBLIC ${Boost_LIBRARIES})