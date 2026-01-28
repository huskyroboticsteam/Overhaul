## Building
In one terminal, `source /opt/ros/humble/setup.bash`

Then, `cd rover_ws && colcon build`

## Running
In a separate terminal from where you built, `source /opt/ros/humble/setup.bash`

Then, `cd rover_ws && source install/local_setup.bash`

Finally, `ros2 run rover Rover --ros-args -r peripheral:=<peripheral>`