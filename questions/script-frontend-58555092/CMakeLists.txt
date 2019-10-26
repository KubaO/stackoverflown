cmake_minimum_required(VERSION 3.5)

project(script-frontend-58555092 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5Widgets)

add_executable(script-frontend-58555092 main.cpp)
target_link_libraries(script-frontend-58555092 Qt5::Widgets)
