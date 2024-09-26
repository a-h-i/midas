
SET(SOURCE_LIST

  momentum_trader.cpp
  trader_data.cpp
)

add_library(trader ${SOURCE_LIST})


target_compile_features(trader PUBLIC cxx_std_23)
target_compile_options(trader PRIVATE -Wall -Wextra -Wpedantic)

target_include_directories(trader PUBLIC ../../include)

find_package(TBB REQUIRED)
target_link_libraries(trader PUBLIC TBB::tbb)


add_dependencies(trader TechnicalAnalysisDownload)
target_link_libraries(trader PRIVATE TechnicalAnalysis)
target_link_libraries(trader PUBLIC broker)
target_link_libraries(trader PUBLIC subscription)

find_package(Boost 1.83 REQUIRED)
target_include_directories(trader PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(trader PUBLIC ${Boost_LIBRARIES})
add_subdirectory(tests)