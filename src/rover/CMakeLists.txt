cmake_minimum_required(VERSION 3.8)
project(rover)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rcl_interfaces REQUIRED)
find_package(rclpy REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(drive_interfaces REQUIRED)
find_package(can REQUIRED)

add_executable(Rover src/Rover.cpp)
add_executable(Drive src/Drive.cpp)
add_executable(WheelMotor src/WheelMotor.cpp)
target_include_directories(WheelMotor PRIVATE include)
target_compile_features(WheelMotor PUBLIC cxx_std_17)
ament_target_dependencies(WheelMotor rclcpp geometry_msgs std_msgs drive_interfaces can)
target_link_libraries(WheelMotor can::CAN26)
target_include_directories(Rover PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

target_include_directories(Drive PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

target_compile_features(Rover PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
ament_target_dependencies(
  Rover
  rclcpp
  rcl_interfaces
  geometry_msgs
)

target_compile_features(Drive PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
ament_target_dependencies(
  Drive
  rclcpp
  rcl_interfaces
  geometry_msgs
  drive_interfaces
)

install(TARGETS
  Rover
  DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS
  Drive
  WheelMotor
  DESTINATION lib/${PROJECT_NAME}
)


if(BUILD_TESTING)
  add_executable(drive_core_test test/drive_core_test.cpp)
  target_include_directories(drive_core_test PRIVATE include)
  target_compile_features(drive_core_test PRIVATE cxx_std_17)
  add_test(NAME drive_core_test COMMAND drive_core_test)
  find_package(ament_cmake_pytest REQUIRED)
  ament_add_pytest_test(drive_ros_test test/test_drive_ros.py TIMEOUT 90)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

# Install launch files
install(DIRECTORY
  launch
  config
  DESTINATION share/${PROJECT_NAME}
)
# Install Python modules
ament_python_install_package(${PROJECT_NAME})

# Install Python executables
install(PROGRAMS
  rover/controller_node.py
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
