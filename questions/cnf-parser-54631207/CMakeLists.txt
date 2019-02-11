cmake_minimum_required(VERSION 2.8)
set(Grammar cnf)
add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${Grammar}.h" "${CMAKE_CURRENT_BINARY_DIR}/${Grammar}.c"
  MAIN_DEPENDENCY "${Grammar}.y"
  COMMAND "cp" "${CMAKE_CURRENT_SOURCE_DIR}/${Grammar}.y" "${CMAKE_CURRENT_BINARY_DIR}"
  COMMAND "/opt/local/bin/lemon" "${Grammar}.y"
)
project(cnf-parser-54631207)
add_executable(
  ${PROJECT_NAME}
  "main.c"
  "${Grammar}.y"
)
add_dependencies(
  ${PROJECT_NAME}
  "${CMAKE_CURRENT_BINARY_DIR}/${Grammar}.c"
)
target_include_directories(
  ${PROJECT_NAME}
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
)
