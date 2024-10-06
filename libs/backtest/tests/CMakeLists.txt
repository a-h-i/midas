
SET(TEST_SRCS 
simulation_order_transmitter_tests.cpp
)


add_executable(backtest_tests ${TEST_SRCS})
target_include_directories(backtest_tests PRIVATE ../include)
target_link_libraries(backtest_tests PRIVATE backtest)
target_link_libraries(backtest_tests PRIVATE gmock_main)
target_include_directories(backtest_tests PRIVATE ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include)
target_include_directories(backtest_tests PRIVATE ${CMAKE_BINARY_DIR}/_deps/googletest-src/googlemock/include)
target_link_libraries(backtest_tests PUBLIC logging)

add_test(NAME backtest_tests COMMAND backtest_tests)
