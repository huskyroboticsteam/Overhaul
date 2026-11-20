ws_dir=$(pwd)
sudo apt install -y build-essential cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
mkdir ~/build_opencv
cd ~/build_opencv
git clone --branch 4.8.0 https://github.com/opencv/opencv
cd opencv
mkdir build && cd build
cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ..
make -j$(nproc)
sudo make install
sudo ldconfig
cd "$ws_dir"
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

