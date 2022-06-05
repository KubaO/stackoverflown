cmake_minimum_required(VERSION 3.8.2)

project(mainwindow-crash-double-delete-58305305 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Widgets AxContainer REQUIRED)

add_executable(${PROJECT_NAME}
    "main.cpp"
    "../../tooling/screenshot.cpp"
    "../../tooling/tooling.cpp"
    "../../tooling/showinshell_win.cpp")
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::AxContainer)
