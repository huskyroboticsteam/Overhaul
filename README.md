# Pre-Setup Notes (Windows)

Our codebase is developed for an NVIDIA Jetson TX2, which runs Ubuntu Linux; as such, much
of our code will be Unix-specific.

> ⚠️ The only supported platform is Ubuntu 22.04 LTS. Other versions/distros/platforms may work, but if you want as easy as possible, go with this version. 

**Windows users:** You should use either [Windows Subsystem for
Linux](https://docs.microsoft.com/en-us/windows/wsl/about) or a VM with a Linux
distribution installed (Ubuntu recommended). Either should work fine. Whichever you use, install either the VM or WSL2 and follow the Linux instructions. As noted above, **use Ubuntu 22.04**.

**From here on out, the installation instructions will assume you are using Ubuntu 22.04 LTS**. Windows users should run commands in either their Linux VM or their WSL
terminal. For Linux users, we'll assume you're running Ubuntu; users of another
distribution may need to change some instructions (e.g. package managers) depending on
your distro.

Read instructions in .devcontainer/README.md first! This will allow all of our builds to be in the same environments
(hopefully)!

> From here instructions assume that you are in a linux environment

## Docker
1. Install Dev Containers extension to vscode
    - Left side bar find extensions and search for Dev Containers. Correct one is by microsoft.
2. Install [Docker Desktop](https://docs.docker.com/desktop/setup/install/windows-install/) on machine
3. Run `Dev Containers: Reopen in Container` in vscode

## Building
In one terminal, `source /opt/ros/humble/setup.bash` (Should be sourced by default but just in case)

Then run the build command in the project root directory, `colcon build`

## Running
In a separate terminal from where you built, `source /opt/ros/humble/setup.bash`

Then, `source install/local_setup.bash`

Finally, `ros2 run rover Rover --ros-args -p peripheral:=<peripheral>`

Running mc node `ros2 run mc --ros-args`

## Adding new ROS2 Packages
Suppose we want to add a custom ROS2 package (i.e rosbag2). Remember to update the following:

1. `package.xml` - Add `<depend>custom_node</depend>`
2. `CMakeLists.txt` - Add `find_package(custom_node REQUIRED)`
3. `CMakeLists.txt` - Add `custom_node` under `ament_target_dependencies`

After adding, run the following:

1. Install external dependencies `rosdep install --from-paths src --ignore-src -y` (as ChatGPT says, "get ingredients")
2. Run CMake `colcon build --packages-select rover` (as ChatGPT says "cook the meal")

## Adding new (external) ROS2 Packages
Add install steps into Dockerfile. For example, add the following to Dockerfile:

`RUN apt-get install -y ros-humble-mavros ros-humble-mavros-extras`

## Miscellaneous Notes
- If the Jetson isn't connecting to the internet, try the following on the host computer:
  - Change "Wired Connected" to 10.40.0.x
  - Go to "Wired Settings" and make sure "Shared to other computers" is checked (under IPv4)
  - Try disabling the firewall with the following command: `sudo ufw disable`. If it works after disabling the firewall, set `DEFAULT_FORWARD_POLICY` to `ACCEPT` in `/etc/default/ufw`. You can re-enable ufw after it is set. 
  - There might also be some DNS issues on the Jetson. Good luck if that's the case. 