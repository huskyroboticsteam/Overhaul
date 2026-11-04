sudo apt install python3-pip
pip3 install dronekit 
rosdep install --from-paths src --ignore-src -y 
colcon build
sudo lsusb
sudo dmesg -c
sudo usermod -aG dialout $USER
newgrp dialout
sudo dmesg -c
colcon build
