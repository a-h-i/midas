
SET(SOURCE_LIST

  momentum_trader.cpp
  trader_data.cpp
  base_trader.cpp
  momentum_factory.cpp
)

add_library(trader ${SOURCE_LIST})
add_dependencies(trader TechnicalAnalysisDownload)

target_compile_features(trader PUBLIC cxx_std_23)
target_compile_options(trader PRIVATE -Wall -Wextra -Wpedantic)

target_include_directories(trader PUBLIC ../../include)
target_include_directories(trader PRIVATE ${TA_INCLUDE_DIR})

find_package(TBB REQUIRED)
target_link_libraries(trader PUBLIC TBB::tbb)



target_link_libraries(trader PRIVATE ta_lib)
target_link_libraries(trader PUBLIC broker)
target_link_libraries(trader PUBLIC subscription)
target_link_libraries(trader PUBLIC order)

find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED)
target_include_directories(trader PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(trader PUBLIC ${Boost_LIBRARIES})
add_subdirectory(tests)