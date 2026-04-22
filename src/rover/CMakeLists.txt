cmake_minimum_required(VERSION 3.8)
project(rover)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rcl_interfaces REQUIRED)

include(FetchContent)

# FetchContent_Declare(
#   foxglove
#   DOWNLOAD_EXTRACT_TIMESTAMP TRUE
#   URL https://github.com/foxglove/foxglove-sdk/releases/download/sdk%2Fv0.17.2/foxglove-v0.17.2-cpp-aarch64-unknown-linux-gnu.zip
#   URL_HASH SHA256=b53f9e1791aa5c48d925f9056ee7ebc4e1519238f70172a2e3960c4b88f8d841
# )
# FetchContent_MakeAvailable(foxglove)

add_executable(Rover src/Rover.cpp src/Globals.cpp)
target_include_directories(Rover PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

file(GLOB CONFIGURE_DEPENDS
)

target_compile_features(Rover PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
ament_target_dependencies(
  Rover
  rclcpp
  rcl_interfaces
)

install(TARGETS Rover
  DESTINATION lib/${PROJECT_NAME})

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

ament_package()
