file(GLOB HEADER_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/broker-interface/*.hpp")


set(SOURCE_LIST
  subscription.cpp

)

add_library(subscription ${SOURCE_LIST} ${HEADER_FILES})
target_compile_features(subscription PUBLIC cxx_std_23)
target_compile_options(subscription PRIVATE -Wall -Wextra -Wpedantic)
# Public interface headers
target_include_directories(subscription PUBLIC ../../include)
# Boost
find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED COMPONENTS date_time REQUIRED)
target_include_directories(subscription PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(subscription PUBLIC ${Boost_LIBRARIES})