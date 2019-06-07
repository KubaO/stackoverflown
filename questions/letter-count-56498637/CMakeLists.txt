cmake_minimum_required(VERSION 2.8)

project(letter-count-56498637)
add_executable(${PROJECT_NAME} "main.cpp")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
configure_file(words_alpha.txt words_alpha.txt COPYONLY)
