
SET(TEST_SRCS trader_data_tests.cpp trader_sampling_tests.cpp base_trader_tests.cpp)

add_executable(trader_tests ${TEST_SRCS})

target_link_libraries(trader_tests PRIVATE trader logging)
target_link_libraries(trader_tests PRIVATE gtest_main gmock_main)
target_include_directories(trader_tests PRIVATE ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include)
target_include_directories(trader_tests PRIVATE ${CMAKE_BINARY_DIR}/_deps/googletest-src/googlemock/include)
# Internal headers
target_include_directories(trader_tests PRIVATE ../include)
add_test(NAME trader_tests COMMAND trader_tests)


SET(MOMENTUM_SRCS momentum_trader_tests.cpp)
add_executable(momentum_trader_tests ${MOMENTUM_SRCS})
target_link_libraries(momentum_trader_tests PRIVATE trader gmock_main logging)
target_include_directories(momentum_trader_tests PRIVATE ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include)
target_include_directories(momentum_trader_tests PRIVATE ${CMAKE_BINARY_DIR}/_deps/googletest-src/googlemock/include)
target_include_directories(momentum_trader_tests PRIVATE ../include)
add_test(NAME momentum_trader_tests COMMAND momentum_trader_tests)