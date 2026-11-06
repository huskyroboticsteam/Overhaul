cmake_minimum_required(VERSION 3.8)
project(imu_gps)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(Threads REQUIRED)


set(UNITREE_SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imu_lidar)
file(GLOB UNITREE_SDK_LIB "${UNITREE_SDK_DIR}/lib/${CMAKE_SYSTEM_PROCESSOR}/*.a")
if(NOT UNITREE_SDK_LIB)
  message(FATAL_ERROR
    "No Unitree SDK library found in ${UNITREE_SDK_DIR}/lib/${CMAKE_SYSTEM_PROCESSOR}")
endif()

# C++ node
add_executable(unitree_lidar_imu
  imu_lidar/publisher.cpp
)

target_include_directories(unitree_lidar_imu PRIVATE
  ${UNITREE_SDK_DIR}/include
)

ament_target_dependencies(unitree_lidar_imu rclcpp geometry_msgs sensor_msgs)
target_link_libraries(unitree_lidar_imu ${UNITREE_SDK_LIB} Threads::Threads)

install(TARGETS unitree_lidar_imu
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

# Python package + script
ament_python_install_package(${PROJECT_NAME})

install(PROGRAMS
  imu_gps/publisher.py
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()