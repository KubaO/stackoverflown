cmake_minimum_required(VERSION 3.1)

project(c-cpp-library-api-53643120)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CSS_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_AUTOMOC ON)

#find_package(Qt5Core)

add_executable(${PROJECT_NAME} "main.cpp" "library.c")

#target_link_libraries(${PROJECT_NAME} Qt5::Core)
