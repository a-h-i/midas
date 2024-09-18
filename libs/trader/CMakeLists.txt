
SET(SOURCE_LIST

  trader.cpp
)

add_library(trader ${SOURCE_LIST})


target_compile_features(trader PUBLIC cxx_std_23)
target_compile_options(trader PRIVATE -Wall -Wextra -Wpedantic)

target_include_directories(trader PUBLIC ../../include)

add_dependencies(trader TechnicalAnalysisDownload)
target_link_libraries(trader PUBLIC )

target_link_libraries(trader PUBLIC broker)
