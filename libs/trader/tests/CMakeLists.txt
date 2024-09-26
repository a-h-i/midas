
SET(TEST_SRCS trader_data_tests.cpp )

add_executable(trader_tests ${TEST_SRCS})

target_link_libraries(trader_tests PRIVATE trader)
target_link_libraries(trader_tests PRIVATE gtest_main)
target_include_directories(trader_tests PRIVATE ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include)
# Internal headers
target_include_directories(trader_tests PRIVATE ../include)
target_link_libraries(trader_tests PRIVATE twsapi)
add_test(NAME trader_tests COMMAND trader_tests)

