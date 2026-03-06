## Building
In one terminal, `source /opt/ros/humble/setup.bash`

Then run the build command in the project root directory, `colcon build`

## Running
In a separate terminal from where you built, `source /opt/ros/humble/setup.bash`

Then, `source install/local_setup.bash`

Finally, `ros2 run rover Rover --ros-args -r peripheral:=<peripheral>`

### Possible issues and possible solutions

When running Colcon build on WSL some people were running into issues regarding the CMake not being able to locate catkin_pkg. There is a install command that utilized conda to resolve this issue although I am not sure about the importance of the other two packages (cona-forge, empy, lark)

conda install -c conda-forge catkin_pkg empy lark
https://github.com/colcon/colcon-ros/issues/118

If you don't have miniconda install via these instructions:
https://www.anaconda.com/docs/getting-started/miniconda/install#manual-shell-initialization