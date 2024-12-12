
SET(TEST_SRCS 
  bar_conversion_tests.cpp 
  transform_order_tests.cpp
  execution_entry_tests.cpp
  build_futures_contract_tests.cpp
)

add_executable(ibkr_tests ${TEST_SRCS})

target_link_libraries(ibkr_tests PRIVATE ibkr_driver)
target_link_libraries(ibkr_tests PRIVATE gmock_main)
target_link_libraries(ibkr_tests PRIVATE logging order)
target_include_directories(ibkr_tests PRIVATE ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include)
# Internal headers
target_include_directories(ibkr_tests PRIVATE ../include)
target_link_libraries(ibkr_tests PRIVATE twsapi)
add_test(NAME ibkr_tests COMMAND ibkr_tests)