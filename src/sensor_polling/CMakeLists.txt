cmake_minimum_required(VERSION 3.8)
project(sensor_polling)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

option(LINK_SHARED_ZED "Link with the ZED SDK shared executable" ON)

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
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)
find_package(OpenCV 4.8 EXACT REQUIRED)
find_package(ZED REQUIRED)
find_package(CUDA ${ZED_CUDA_VERSION} REQUIRED)
find_package(zed_msgs REQUIRED)

include_directories(${CUDA_INCLUDE_DIRS})
include_directories(${ZED_INCLUDE_DIRS})

link_directories(${ZED_LIBRARY_DIR})
link_directories(${CUDA_LIBRARY_DIRS})

set(UNITREE_SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lidar)
file(GLOB UNITREE_SDK_LIB "${UNITREE_SDK_DIR}/lib/${CMAKE_SYSTEM_PROCESSOR}/*.a")
if(NOT UNITREE_SDK_LIB)
  message(FATAL_ERROR
    "No Unitree SDK library found in ${UNITREE_SDK_DIR}/lib/${CMAKE_SYSTEM_PROCESSOR}")
endif()

# C++ node
add_executable(unitree_lidar
  lidar/publisher.cpp
)

add_executable(basic_cam
  cameras/basic_cam/publisher.cpp
)

add_executable(zed_cam
  cameras/zed_cam/publisher.cpp
  cameras/zed_cam/cam_publisher.cpp
  cameras/zed_cam/params.cpp
)

target_include_directories(unitree_lidar PRIVATE
  ${UNITREE_SDK_DIR}/include
)

if (LINK_SHARED_ZED)
    set(ZED_LIBS ${ZED_LIBRARIES} ${CUDA_CUDA_LIBRARY} ${CUDA_CUDART_LIBRARY})
else()
    set(ZED_LIBS ${ZED_STATIC_LIBRARIES} ${CUDA_CUDA_LIBRARY} ${CUDA_LIBRARY})
endif()

ament_target_dependencies(zed_cam rclcpp sensor_msgs std_msgs geometry_msgs zed_msgs cv_bridge image_transport OpenCV)
target_link_libraries(zed_cam ${ZED_LIBS} Threads::Threads)

ament_target_dependencies(unitree_lidar rclcpp geometry_msgs sensor_msgs)
target_link_libraries(unitree_lidar ${UNITREE_SDK_LIB} Threads::Threads)

ament_target_dependencies(basic_cam rclcpp sensor_msgs cv_bridge image_transport OpenCV Threads)


install(TARGETS unitree_lidar
  DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS basic_cam
  DESTINATION lib/${PROJECT_NAME}
)

install (TARGETS zed_cam
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
  sensor_polling/pixhawk/publisher.py
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
