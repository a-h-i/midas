
SET(SOURCE_LIST

  exchange.cpp
  timezones.cpp
  data_stream.cpp
  data_stream_export.cpp
)

add_library(broker ${SOURCE_LIST})


target_compile_features(broker PUBLIC cxx_std_23)
target_compile_options(broker PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(broker PUBLIC ../../include)


target_link_libraries(broker PUBLIC ibkr_driver)

# Boost
find_package(Boost 1.83 COMPONENTS date_time REQUIRED)

target_include_directories(broker PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(broker PUBLIC ${Boost_LIBRARIES})

add_subdirectory(tests)