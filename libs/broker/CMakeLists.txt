
SET(SOURCE_LIST
  timezones.cpp
  data_stream.cpp
  data_stream_export.cpp
  bar_import.cpp
)

add_library(broker ${SOURCE_LIST})


target_compile_features(broker PUBLIC cxx_std_23)
target_compile_options(broker PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(broker PUBLIC ../../include)


target_link_libraries(broker PUBLIC ibkr_driver)

# Boost
find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED COMPONENTS date_time REQUIRED)

target_include_directories(broker PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(broker PUBLIC ${Boost_LIBRARIES})
target_link_libraries(broker PUBLIC subscription)

add_subdirectory(tests)