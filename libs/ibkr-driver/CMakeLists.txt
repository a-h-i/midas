
file(GLOB HEADER_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/broker/*.hpp")

set(SOURCE_LIST 
  ibkr.cpp client.cpp errors.cpp order.cpp 
  account.cpp contract.cpp
  ticks.cpp
  data_requests.cpp
  build_contracts.cpp
  connectivity_state.cpp
  subscriptions.cpp
  subscription.cpp
  bar_conversion.cpp
)

add_library(ibkr_driver ${SOURCE_LIST} ${HEADER_FILES})


target_compile_features(ibkr_driver PUBLIC cxx_std_23)
target_compile_options(ibkr_driver PRIVATE -Wall -Wextra -Wpedantic)

# Public interface headers
target_include_directories(ibkr_driver PUBLIC ../../include)
# Internal headers
target_include_directories(ibkr_driver PRIVATE include)




# ibkr tws api
target_link_libraries(ibkr_driver PRIVATE twsapi)
add_dependencies(ibkr_driver IntelFloatingPointDownload)

# intel decimal floating point libs
target_link_libraries(ibkr_driver PUBLIC IntelFloatingPoint)
# Thread support
find_package(Threads REQUIRED)
target_link_libraries(ibkr_driver PUBLIC Threads::Threads)


# Boost
find_package(Boost 1.83 COMPONENTS system log REQUIRED)

target_include_directories(ibkr_driver PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(ibkr_driver PUBLIC ${Boost_LIBRARIES})

target_link_libraries(ibkr_driver INTERFACE exceptions)
