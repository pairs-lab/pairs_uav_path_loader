cmake_minimum_required(VERSION 3.8)
project(pairs_uav_path_loader)

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Warnings
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra)
endif()

## --------------------------------------------------------------
## |                       Dependencies                         |
## --------------------------------------------------------------
set(DEPENDENCIES
  rclcpp
  rclcpp_components
  pairs_msgs
  Eigen3
  pairs_lib
)

foreach(dep IN LISTS DEPENDENCIES)
  find_package(${dep} REQUIRED)
endforeach()

## --------------------------------------------------------------
## |                     Include dirs                           |
## --------------------------------------------------------------
include_directories(
  include
  ${EIGEN3_INCLUDE_DIRS}
  ${pairs_lib_INCLUDE_DIRS}
)

## --------------------------------------------------------------
## |                           Compile                          |
## --------------------------------------------------------------
add_library(path_loader_component SHARED
  src/path_loader.cpp        # <-- поменяй на path_loader_node.cpp если так называется файл
)

target_include_directories(path_loader_component PUBLIC
  include
  ${EIGEN3_INCLUDE_DIRS}
)

ament_target_dependencies(path_loader_component
  ${DEPENDENCIES}
)

# Register component & create executable wrapper
rclcpp_components_register_node(
  path_loader_component
  PLUGIN "path_loader::PathLoaderNode"
  EXECUTABLE path_loader_node_component
)

## --------------------------------------------------------------
## |                           Install                          |
## --------------------------------------------------------------
install(
  TARGETS path_loader_component path_loader_node_component
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  DIRECTORY launch paths
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
