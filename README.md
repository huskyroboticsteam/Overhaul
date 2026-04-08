Read instructions in .devcontainer/README.md first! This will allow all of our builds to be in the same environments
(hopefully)!

## Building
In one terminal, `source /opt/ros/humble/setup.bash` (Should be sourced by default but just in case)

Then run the build command in the project root directory, `colcon build`

## Running
In a separate terminal from where you built, `source /opt/ros/humble/setup.bash`

Then, `source install/local_setup.bash`

Finally, `ros2 run rover Rover --ros-args -p peripheral:=<peripheral>`