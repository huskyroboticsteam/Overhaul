sudo apt install python3-pip
pip3 install dronekit 
rosdep install --from-paths src --ignore-src -y 
colcon build