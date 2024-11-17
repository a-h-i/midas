
file(GLOB HEADER_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/broker/*.hpp")
file(GLOB INTERNAL_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/ibkr/internal/*.hpp")

set(SOURCE_LIST 
  client.cpp errors.cpp order.cpp 
  account.cpp contract.cpp
  ticks.cpp
  build_contracts.cpp
  connectivity_state.cpp
  subscriptions.cpp
  bar_conversion.cpp
  driver.cpp
  active_sub_state.cpp
  order_manager.cpp
  transform_order.cpp
  historical_data.cpp
  native_order.cpp
)
add_library(ibkr_driver ${SOURCE_LIST} ${HEADER_FILES} ${INTERNAL_HEADERS})


target_compile_features(ibkr_driver PUBLIC cxx_std_23)
target_compile_options(ibkr_driver PRIVATE -Wall -Wextra -Wpedantic)

# Public interface headers
target_include_directories(ibkr_driver PUBLIC ../../include)
# Internal headers
target_include_directories(ibkr_driver PRIVATE include)




# ibkr tws api
target_link_libraries(ibkr_driver PUBLIC twsapi)
add_dependencies(ibkr_driver IntelFloatingPointDownload)

# intel decimal floating point libs
target_link_libraries(ibkr_driver PRIVATE IntelFloatingPoint)
# Thread support
find_package(Threads REQUIRED)
target_link_libraries(ibkr_driver PUBLIC Threads::Threads)


# Boost
find_package(Boost 1.83 COMPONENTS system log REQUIRED)

target_include_directories(ibkr_driver PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(ibkr_driver PUBLIC ${Boost_LIBRARIES})

target_link_libraries(ibkr_driver INTERFACE exceptions)
target_link_libraries(ibkr_driver PUBLIC subscription)

add_subdirectory(tests)