cmake_minimum_required(VERSION 3.8)
project(h1_fullbody_controller)

# rclcpp requires C++17 (std::clamp in walk_straight.cpp does too).
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclpy REQUIRED)
find_package(rclcpp REQUIRED)          # added: C++ client library
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)

# ---------------------------------------------------------------------------
# C++ nodes
# ---------------------------------------------------------------------------
add_executable(walk_straight src/walk_straight.cpp)
ament_target_dependencies(walk_straight
  rclcpp
  geometry_msgs
)

install(TARGETS
  walk_straight
  DESTINATION lib/${PROJECT_NAME}
)

add_executable(walk_circle src/walk_circle.cpp)
ament_target_dependencies(walk_circle
  rclcpp
  geometry_msgs
)

install(TARGETS
  walk_circle
  DESTINATION lib/${PROJECT_NAME}
)

# ---------------------------------------------------------------------------
# Data directories and Python scripts (unchanged from the original)
# ---------------------------------------------------------------------------
install(DIRECTORY
  scripts
  launch
  policy
  DESTINATION share/${PROJECT_NAME})

# Install Python executables
install(PROGRAMS
  scripts/h1_fullbody_controller.py
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()