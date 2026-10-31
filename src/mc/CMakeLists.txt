cmake_minimum_required(VERSION 3.8)
project(mc)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rcl_interfaces REQUIRED)
find_package(rosidl_default_generators REQUIRED)

# Create custom .msg/.srv into usable interfaces
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/MCPacket.msg"
  # DEPENDENCIES
)

rosidl_get_typesupport_target(cpp_typesupport_target "${PROJECT_NAME}" "rosidl_typesupport_cpp")

add_executable(MissionControl src/MissionControlProtocol.cpp)
target_include_directories(MissionControl PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

target_compile_features(MissionControl PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
ament_target_dependencies(
  MissionControl
  rclcpp
  rcl_interfaces
)

target_link_libraries(MissionControl ${cpp_typesupport_target})

install(TARGETS
  MissionControl
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
