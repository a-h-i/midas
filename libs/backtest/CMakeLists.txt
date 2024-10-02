file(GLOB HEADER_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/backtest/*.hpp")

set(SOURCE_LIST
  backtest.cpp
  operators.cpp
)

add_library(backtest  ${SOURCE_LIST} ${HEADER_FILES})
target_compile_features(backtest PUBLIC cxx_std_23)
target_compile_options(backtest PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(backtest PUBLIC ../../include)
target_include_directories(backtest PRIVATE ./include)