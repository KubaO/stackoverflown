cmake_minimum_required(VERSION 3.5)

project(qgraphicsitem-brush-62028912 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5Widgets)

add_executable(qgraphicsitem-brush-62028912 main.cpp)
target_link_libraries(qgraphicsitem-brush-62028912 Qt5::Widgets)
