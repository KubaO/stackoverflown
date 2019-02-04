cmake_minimum_required(VERSION 3.2)

set(Python_ADDITIONAL_VERSIONS 3.6)
project(swig-pair)
find_package(SWIG 3.0 REQUIRED)
find_package(PythonLibs 3.6 REQUIRED)
include(UseSwig)

SWIG_MODULE_INITIALIZE(${PROJECT_NAME} python)
SWIG_ADD_SOURCE_TO_MODULE(${PROJECT_NAME} swig_generated_sources "token_mod.i")
add_executable(${PROJECT_NAME} "main.c" ${swig_generated_sources})
target_include_directories(${PROJECT_NAME} PRIVATE ${PYTHON_INCLUDE_DIRS} ".")
target_link_libraries(${PROJECT_NAME} PRIVATE ${PYTHON_LIBRARIES})
