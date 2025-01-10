
SET(TEST_SRCS
        bar_test.cpp
        position_tracker_tests.cpp
)

add_executable(broker_tests ${TEST_SRCS})

target_link_libraries(broker_tests PRIVATE broker)
target_link_libraries(broker_tests PRIVATE gtest_main)
target_include_directories(broker_tests PRIVATE ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include)

add_test(NAME broker_tests COMMAND broker_tests)

