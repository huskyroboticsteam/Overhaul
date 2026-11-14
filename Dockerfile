# Rover image: the Overhaul ROS 2 packages, built and ready to launch.
# The devcontainer (.devcontainer/Dockerfile) is for development; this one is built by CI.
FROM ros:humble-ros-base

SHELL ["/bin/bash", "-o", "pipefail", "-c"]

WORKDIR /ros2_ws
COPY src/ src/

# Install package dependencies (including test deps so CI can run colcon test), then build
RUN apt-get update -q \
    && rosdep update --rosdistro "$ROS_DISTRO" \
    && rosdep install --from-paths src --ignore-src -y \
    && rm -rf /var/lib/apt/lists/* \
    && source /opt/ros/"$ROS_DISTRO"/setup.bash \
    && colcon build

COPY --chmod=755 docker/entrypoint.sh /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
CMD ["ros2", "launch", "rover", "rover_launch.xml"]
