file(GLOB HEADER_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/broker-interface/*.hpp")


set(SOURCE_LIST
  order.cpp
  simple_order.cpp
  bracketed_order.cpp
  operators.cpp
)

add_library(order ${SOURCE_LIST} ${HEADER_FILES})
target_compile_features(order PUBLIC cxx_std_23)
target_compile_options(order PRIVATE -Wall -Wextra -Wpedantic)
# Public interface headers
target_include_directories(order PUBLIC ../../include)
target_include_directories(order PRIVATE ./include)
# Boost
find_package(Boost 1.83 COMPONENTS date_time REQUIRED)
target_include_directories(order PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(order PUBLIC ${Boost_LIBRARIES})