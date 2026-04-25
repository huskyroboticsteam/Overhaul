### Docker instructions

Prereqs:
 - using vscode
 - download extension "Dev Containers"

The instructions are based on the tutorial online https://docs.ros.org/en/humble/How-To-Guides/Setup-ROS-2-with-VSCode-and-Docker-Container.html#id7

CLI instructions:

From this directory you can build via `docker build .`.
### Docker over terminal
For those who don't want to deal with vs code (or it's mainly running headless on Jetson) the following instructions include setting up as well.

1. 'sudo apt install docker.io'
2. 'cd into /Overhaul/.devcontainer folder'
3. 'sudo docker build . -t overhaul-img'. You can verify that the image exist
4. 'sudo docker run -it --mount source=~/Overhaul,target=/home/ws,type=bind overhaul-img'
5. 'cd /home/ws'
