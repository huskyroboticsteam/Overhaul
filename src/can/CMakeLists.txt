cmake_minimum_required(VERSION 3.8)
project(can)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rcl_interfaces REQUIRED)
find_package(rosidl_default_generators REQUIRED)

include(FetchContent)
FetchContent_Declare(
  CAN26
  GIT_REPOSITORY https://github.com/huskyroboticsteam/CAN26.git
  GIT_TAG 0aee8a7b4de9c09b57fc47f48bf4d0837d12f340
)
FetchContent_MakeAvailable(CAN26)
add_library(CAN26 STATIC ${can26_SOURCE_DIR}/CANPacket.c)
target_include_directories(CAN26 SYSTEM PUBLIC ${can26_SOURCE_DIR})

# Create custom .msg/.srv into usable interfaces
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/CANDevice.msg"
  "msg/CANPacket.msg"
  # DEPENDENCIES
)
rosidl_get_typesupport_target(cpp_typesupport_target "${PROJECT_NAME}" "rosidl_typesupport_cpp")

add_executable(CANReceiver
  src/CANReceiver.cpp)
add_executable(CANSender
  src/CANSender.cpp)

target_include_directories(CANReceiver PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_include_directories(CANSender PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

target_link_libraries(CANReceiver ${cpp_typesupport_target} CAN26)
target_link_libraries(CANSender ${cpp_typesupport_target} CAN26)

target_compile_features(CANReceiver PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
target_compile_features(CANSender PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17

ament_target_dependencies(
  CANReceiver
  rclcpp
  rcl_interfaces
)
ament_target_dependencies(
  CANSender
  rclcpp
  rcl_interfaces
)

install(TARGETS
  CANReceiver
  DESTINATION lib/${PROJECT_NAME}
)
install(TARGETS
  CANSender
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
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

ament_export_dependencies(rosidl_default_runtime)
ament_package()
