sudo apt install python3-pip
pip3 install dronekit pyserial
sudo apt install libopencv-dev python3-opencv ros-humble-cv-bridge ros-humble-image-transport
rosdep install --from-paths src --ignore-src -y 
colcon build
sudo lsusb
sudo dmesg -c
sudo usermod -aG dialout $USER
newgrp dialout
sudo dmesg -c
colcon build