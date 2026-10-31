FROM ros:humble

ARG DEBIAN_FRONTEND=noninteractive

ARG USERNAME=ros
ARG USER_UID=1000
ARG USER_GID=$USER_UID

SHELL ["/bin/bash", "-o", "pipefail", "-c"]

# Delete user if it exists in container (e.g Ubuntu Noble: ubuntu)
RUN if getent passwd "$USER_UID"; then \
        userdel "$(getent passwd "$USER_UID" | cut -d: -f1)"; \
    fi

# Create the user
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    #
    # [Optional] Add sudo support. Omit if you don't need to install software after connecting.
    && apt-get update \
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME \
    && rm -rf /var/lib/apt/lists/*

# Install dependencies and tools
RUN apt-get update \
    && apt-get install -y \
    python3-pip \
    vim \
    && rm -rf /var/lib/apt/lists/*

# Install Husky Robotics dependencies
RUN apt-get update \
    && apt-get install -y \
    curl \
    wget \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y \
    xfce4 \
    xfce4-terminal \
    tigervnc-standalone-server \
    tigervnc-common \
    dbus-x11 \
    fonts-liberation \
    xfonts-base \
    && rm -rf /var/lib/apt/lists/*

# Transfer key
RUN mkdir -p /etc/apt/keyrings/
RUN wget -P /etc/apt/keyrings/ "https://huskyroboticsteam.github.io/ubuntu-repo/KEY.gpg"
RUN gpg --no-default-keyring --keyring ./temp-keyring.gpg --import /etc/apt/keyrings/KEY.gpg
RUN gpg --no-default-keyring --keyring ./temp-keyring.gpg --export --output /etc/apt/keyrings/KEY.gpg --yes
RUN rm temp-keyring.gpg*
RUN wget -P /etc/apt/sources.list.d "https://huskyroboticsteam.github.io/ubuntu-repo/husky_robotics.list"

# Install Gazebo ROS Deps
RUN curl https://packages.osrfoundation.org/gazebo.gpg --output /usr/share/keyrings/pkgs-osrf-archive-keyring.gpg
RUN echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/pkgs-osrf-archive-keyring.gpg] https://packages.osrfoundation.org/gazebo/ubuntu-stable $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/gazebo-stable.list > /dev/null
RUN apt-get update
RUN apt-get install -y ignition-fortress

# Set up VNC password (the password for connecting via VNC client)
# The echo string to xstartup is because the default xstartup does not want to work with the docker container
# why? feel free to debug
RUN mkdir -p /home/${USERNAME}/.vnc && \
    echo "dockervnc" | vncpasswd -f > /home/${USERNAME}/.vnc/passwd && \
    chmod 600 /home/${USERNAME}/.vnc/passwd && \
    printf '#!/bin/sh\n\
    unset WAYLAND_DISPLAY\n\
    unset WAYLAND_SOCKET\n\
    unset SESSION_MANAGER\n\
    unset DBUS_SESSION_BUS_ADDRESS\n\
    export DISPLAY=:1\n\
    exec dbus-launch --exit-with-session startxfce4\n' > /home/${USERNAME}/.vnc/xstartup && \
    chmod +x /home/${USERNAME}/.vnc/xstartup && \
    chown -R ${USERNAME}:${USERNAME} /home/${USERNAME}/.vnc

# Expose VNC port
EXPOSE 5901

# Expose Foxglove bridge port
EXPOSE 8765

RUN apt-get update \
    && apt-get install -y \
    build-essential \
    catch2 \
    cmake \
    frozen \
    git \
    gnupg2 \
    gpsd \
    gpsd-clients \
    libargparse-dev \
    libavutil-dev \
    libboost-system-dev \
    libeigen3-dev \
    libgps-dev \
    libopencv-contrib-dev \
    libopencv-dev=4.5.4+dfsg-9ubuntu4 \
    libwebsocketpp-dev \
    libx264-dev \
    lsb-release \
    nlohmann-json3-dev \
    ublox-linux \
    unzip \
    ros-humble-foxglove-bridge \
    ros-humble-robot-state-publisher \
    ros-humble-ros-gz \
    ros-humble-xacro \
    && rm -rf /var/lib/apt/lists/*

ENV SHELL=/bin/bash

# ********************************************************
# * Anything else you want to do like clean up goes here *
# ********************************************************

# Install MAVROS packages
# RUN apt-get install -y ros-humble-mavros ros-humble-mavros-extras

# Install GeographicLib datasets (MAVROS requirement)
# RUN wget https://raw.githubusercontent.com/mavlink/mavros/ros2/mavros/scripts/install_geographiclib_datasets.sh \
#     && chmod +x install_geographiclib_datasets.sh \
#     && ./install_geographiclib_datasets.sh \
#     && rm install_geographiclib_datasets.sh

# [Optional] Set the default user. Omit if you want to keep the default as root.
USER $USERNAME

# Update bashrc to use alias's, sourcing etc.
# Source ROS, and the workspace once it has been built
RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc \
    && echo '[ -f /home/ws/install/setup.bash ] && source /home/ws/install/setup.bash' >> ~/.bashrc

# Start VNC server on display :1 (port 5901)
CMD ["vncserver", ":1", "-geometry", "1280x720", "-depth", "24", "-localhost", "no", "-fg"]