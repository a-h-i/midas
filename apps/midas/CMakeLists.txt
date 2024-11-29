SET(SOURCES_LIST
  main.cpp
  ui_main_loop.cpp
  trader_view.cpp
  backtest.cpp
  broker_factory.cpp
  profit_and_loss_window.cpp
)

file(GLOB HEADER_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.hpp")
add_executable(midas ${SOURCES_LIST} ${HEADER_FILES} ${PROJECT_SOURCE_DIR}/include/midas/version.h.in )
target_compile_features(midas PRIVATE cxx_std_23 )
target_compile_options(midas PRIVATE -Wall -Wextra -Wpedantic -Werror)


find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED COMPONENTS program_options log REQUIRED)

target_include_directories(midas PRIVATE "HEADERS ${Boost_INCLUDE_DIR}")
target_link_libraries(midas PRIVATE ${Boost_LIBRARIES})

target_include_directories(midas PRIVATE ${PROJECT_BINARY_DIR}/include )
target_include_directories(midas PRIVATE ./include )

target_link_libraries(midas PRIVATE logging)
target_link_libraries(midas PRIVATE broker)
target_link_libraries(midas PRIVATE trader)
target_link_libraries(midas PRIVATE backtest)
target_link_libraries(midas PRIVATE signal_handling)
target_link_libraries(midas PRIVATE ftxui::screen ftxui::dom ftxui::component)