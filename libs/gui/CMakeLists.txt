file(GLOB HEADER_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/gui/*.hpp")
file(GLOB INTERNAL_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)
set(SOURCE_LIST
        start_gui.cpp
        main_window.cpp
        new_trader_dialog.cpp
        trader_widget.cpp
        backtest_widget.cpp
        pnl_widget.cpp
)
find_package(Qt6 REQUIRED COMPONENTS Widgets Core)


add_library(gui ${SOURCE_LIST} ${HEADER_FILES} ${INTERNAL_HEADERS})
set_target_properties(gui PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)
target_compile_features(gui PUBLIC cxx_std_23)
target_compile_options(gui PRIVATE -Wall -Wextra -Wpedantic -Werror)

target_include_directories(gui PUBLIC ../../include)
target_include_directories(gui PRIVATE ./include)
target_link_libraries(gui PUBLIC Qt6::Widgets Qt6::Core trader broker backtest ibkr_driver)

