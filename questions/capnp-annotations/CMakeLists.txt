cmake_minimum_required(VERSION 3.5)
project("capnproto-annotations" CXX)

find_package(CapnProto 0.7.0 REQUIRED)

capnp_generate_cpp(capnpSources capnpHeaders annotations.capnp)

add_executable(${CMAKE_PROJECT_NAME} main.cpp ${capnpSources})
#target_compile_definitions(classtest PRIVATE RE_SPLIT)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE CapnProto::capnp)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
