Read instructions in .devcontainer/README.md first! This will allow all of our builds to be in the same environments
(hopefully)!

## Docker
1. Install Dev Containers extension to vscode 
2. Install docker on your device
3. Run `Dev Containers: Reopen in Container` in vscode

## Building
In one terminal, `source /opt/ros/humble/setup.bash` (Should be sourced by default but just in case)

Then run the build command in the project root directory, `colcon build`

## Running
In a separate terminal from where you built, `source /opt/ros/humble/setup.bash`

Then, `source install/local_setup.bash`

Finally, `ros2 run rover Rover --ros-args -p peripheral:=<peripheral>`

## Adding new ROS2 Packages
Suppose we want to add a ROS2 package (i.e rosbag2). Remember to update the following:

1. `package.xml` - Add `<depend>rosbag2</depend>`
2. `CMakeLists.txt` - Add `find_package(rosbag2 REQUIRED)`
3. `CMakeLists.txt` - Add `rosbag2` under `ament_target_dependencies`

After adding, run the following:

1. Install external dependencies `rosdep install --from-paths src --ignore-src -y` (as ChatGPT says, "get ingredients")
2. Run CMake `colcon build --packages-select rover` (as ChatGPT says "cook the meal")