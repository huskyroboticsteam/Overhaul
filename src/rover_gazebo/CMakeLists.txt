cmake_minimum_required(VERSION 3.8)
project(rover_gazebo)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)

add_executable(drive_test src/drive_test.cpp)
target_compile_features(drive_test PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
ament_target_dependencies(
  drive_test
  rclcpp
  geometry_msgs
)

install(TARGETS
  drive_test
  DESTINATION lib/${PROJECT_NAME}
)

# Install launch, config and simulation files
install(DIRECTORY
  config
  launch
  urdf
  worlds
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
