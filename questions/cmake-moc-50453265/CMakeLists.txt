cmake_minimum_required(VERSION 2.8.12)

# \brief adds for the given target a fake executable targets which allows all
#        headers and symbols to be shown in IDEs.
# \param target_name Which target properties should be added to the IDE support target.
function(target_add_ide_support target_name)
  # from https://github.com/sauter-hq/cmake-ide-support
  if (NOT TARGET ${target_name})
    message(FATAL_ERROR "No target defined with name ${target_name}, cannot target_add_ide_support it.")
  endif()

  set (target_for_ide "${target_name}_ide_support")
  if (NOT TARGET ${target_for_ide})
      file(GLOB_RECURSE target_for_ide_srcs "*.h" "*.hpp" "*.hxx" "*.c" "*.cpp" "*.cxx")
      #add_executable(${target_for_ide} ${target_for_ide_srcs})
      add_custom_target(${target_for_ide} SOURCES ${target_for_ide_srcs})
      set_target_properties(${target_for_ide} PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
  endif()

  get_target_property(dirs ${target_name} INCLUDE_DIRECTORIES)
  if (dirs)
    target_include_directories(${target_for_ide} PRIVATE ${dirs})
  endif()
endfunction(target_add_ide_support)

project(cmake-moc-50453265)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core)

add_executable(${PROJECT_NAME} "main.cpp")
target_add_ide_support(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} Qt5::Core)

unset(QT_QMAKE_EXECUTABLE)
