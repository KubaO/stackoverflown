cmake_minimum_required(VERSION 2.8)

set(UseSWIG_TARGET_NAME_PREFERENCE STANDARD)

project(swig-pair-53915787)
find_package(SWIG 3.0 REQUIRED)
find_package(PythonLibs 2.7 REQUIRED)
include(UseSwig)

swig_add_library(token TYPE STATIC LANGUAGE Python SOURCES "token_mod.i" )
target_include_directories(token PUBLIC ${PYTHON_INCLUDE_DIRS} )
target_link_libraries(token PUBLIC ${PYTHON_LIBRARIES} )

add_executable(${PROJECT_NAME} "main.c")
target_link_libraries(${PROJECT_NAME} token)

