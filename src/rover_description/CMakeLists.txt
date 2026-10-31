cmake_minimum_required(VERSION 3.8)
project(rover_description)

# find dependencies
find_package(ament_cmake REQUIRED)

# Install launch and URDF files
install(DIRECTORY
  launch
  urdf
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
