cmake_minimum_required(VERSION 3.8)
project(imu_gps)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED) # <-- Add this package
find_package(rclcpp REQUIRED)
find_package(rcl_interfaces REQUIRED)
find_package(rclpy REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)




if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

# Explicitly install the python package using a specific subdirectory parameter
# to avoid target collisions with the auto-generated message code
ament_python_install_package(${PROJECT_NAME})

install(PROGRAMS
  imu_gps/publisher.py
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
