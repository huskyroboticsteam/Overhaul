# Pre-Setup Notes

Our codebase is developed for an NVIDIA Jetson TX2, which runs Ubuntu Linux; as such, much
of our code will be Unix-specific.

> ⚠️ The only supported platform is Ubuntu 22.04 LTS. Other versions/distros/platforms may work, but if you want as easy as possible, go with this version. 

We will be using vmware workstation for Windows, or UTM for Mac (For additional support, this installation process is based on [CSE 478 virtual machine setup guide](https://courses.cs.washington.edu/courses/cse478/26sp/projects/introduction/#optional-setting-up-the-virtual-machine) however please refer to instructions in this readme for specific files such as os images).

1. First install the VM
    - **Windows**: vmware workstation, setup an account and install the [desktop application](https://support.broadcom.com/c/portal/login?redirect=%2Fgroup%2Fecx%2Fproductdownloads%3Fsubfamily%3DVMware%2520Workstation%2520Pro%26freeDownloads%3Dtrue&refererPlid=36&p_l_id=33047). 
    - **Mac**: UTM, click download from their [page](https://mac.getutm.app/).
2. Install the [OS image](https://releases.ubuntu.com/jammy/) to your laptop.
3. Finish all of the install instructions for the ubuntu install (This will take a long time so go do some homework or look through the repo).
4. Install basic requirements
        - `sudo apt install git`

> Never update your VM! Ubuntu will try to do it for some reason, just don't do it. Same goes for os on laptops

<<<<<<< HEAD
=======
**Next open a terminal and enter the Overhaul folder within this repo**

```bash
cd \Overhaul
```

**Then build the docker container**

<kbd>ctrl</kbd> + <kbd>shift</kbd> + <kbd>p</kbd>
and search *Rebuild and Reopen in containers*

> Normally just do *Reopen in containers* so you don't have to build every time

**Running into the `failed to connect to the docker API at npipe:////./pipe/dockerDesktopLinuxEngine;`??**
Enter the docker desktop app and go to the top right setting icon. Then enter the Resources tab on the left. Disable integration with WSL distro, save and restart in the bottom corner. Then recheck the enable integration with my defaul WSL distro. Rebuild and reopen and it should work! 

Then wait for a really long time until it lets you type to exit. In the top bar of vs code click terminal and hit new terminal. **At this point you are in the VM and can start developing!**

***Windows VNC***

**If you need a graphical interface (you most likely will) you will need to do some more setup**

First instal the VNC viewer application. Any works however I personally use [this one](([https://www.realvnc.com/en/](https://www.realvnc.com/en/connect/download/viewer/?lai_sr=5-9&lai_sl=l&lai_na=3339)))


Everytime you boot up the docker container you must follow the steps above to create a new terminal. Once in that terminal run the 

```bash
./start_vnc.sh
```

Then Open up real VNC
1. If you have not made a connection to the session yet right click the background and click *new connection*
2. Type in the address 127.0.0.1:5901, you can name the vnc session anything (mine is HRT_Container)
3. connect to it and enter the password *dockervnc*
4. It should connect to the session and you can now see the desktop!


>>>>>>> 737539adc12b8b1247fc35c347133bed8d0934c7
**From here on out, the installation instructions will assume you are using Ubuntu 22.04 LTS**. Windows users should run commands in either their Linux VM or their WSL
terminal. For Linux users, we'll assume you're running Ubuntu; users of another
distribution may need to change some instructions (e.g. package managers) depending on
your distro. Mac users can you run set of instructions in their local terminal or VM.

Read instructions in [.devcontainer/README.md](.devcontainer/README.md) first! This will allow all of our builds to be in the same environments
(hopefully)!

> From here instructions assume that you are in a linux environment

## Docker
1. Install Dev Containers extension to vscode
    - Left side bar find extensions and search for Dev Containers. Correct one is by microsoft.
2. Install 
    - [Window](https://docs.docker.com/desktop/setup/install/windows-install/)
    - [Linux](https://docs.docker.com/desktop/setup/install/linux/)
    - [Mac](https://docs.docker.com/desktop/setup/install/mac-install/)
3. Run `Dev Containers: Reopen in Container` in vscode command palette

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

1. Install external dependencies `rosdep install --from-paths src --ignore-src -y` (as ChatGPT says, "get ingredients 🛒✨")
2. Run CMake `colcon build --packages-select rover` (as ChatGPT says "cook the meal 🍳✨")

## Adding new (external) ROS2 Packages
Add install steps into Dockerfile. For example, add the following to Dockerfile:

`RUN apt-get install -y ros-humble-mavros ros-humble-mavros-extras`

## Miscellaneous Notes
- If the Jetson isn't connecting to the internet, try the following on the host computer:
  - Change "Wired Connected" to 10.40.0.x
  - Go to "Wired Settings" and make sure "Shared to other computers" is checked (under IPv4)
  - Try disabling the firewall with the following command: `sudo ufw disable`. If it works after disabling the firewall, set `DEFAULT_FORWARD_POLICY` to `ACCEPT` in `/etc/default/ufw`. You can re-enable ufw after it is set. 
  - There might also be some DNS issues on the Jetson. Good luck if that's the case. 