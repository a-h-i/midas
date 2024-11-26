SET(SOURCES_LIST
  main.cpp
  ui.cpp
  trader_view.cpp
)
add_executable(midas ${SOURCES_LIST} ${PROJECT_SOURCE_DIR}/include/midas/version.h.in )

target_compile_features(midas PRIVATE cxx_std_23 )
target_compile_options(midas PRIVATE -Wall -Wextra -Wpedantic -Werror)


find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED COMPONENTS program_options log REQUIRED)

target_include_directories(midas PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(midas PRIVATE ${Boost_LIBRARIES})

target_include_directories(midas PRIVATE ${PROJECT_BINARY_DIR}/include )
target_include_directories(midas PRIVATE ./include )

target_link_libraries(midas PRIVATE logging)
target_link_libraries(midas PRIVATE broker)
target_link_libraries(midas PRIVATE trader)
target_link_libraries(midas PRIVATE backtest)
target_link_libraries(midas PRIVATE ftxui::screen ftxui::dom ftxui::component)