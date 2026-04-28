### Docker instructions

Prereqs:
 - using vscode
 - download extension "Dev Containers"

The instructions are based on the tutorial online https://docs.ros.org/en/humble/How-To-Guides/Setup-ROS-2-with-VSCode-and-Docker-Container.html#id7

CLI instructions:

From this directory you can build via `docker build .`.
run the container using the `-p 3002:3002` flag to expose the websocket port
