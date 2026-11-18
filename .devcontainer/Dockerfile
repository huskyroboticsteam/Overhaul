FROM ros:humble

ARG DEBIAN_FRONTEND=noninteractive

ARG USERNAME=ros
ARG USER_UID=1000
ARG USER_GID=$USER_UID

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
    # HRT dependencies
    curl \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Install Gazebo ROS Deps
# RUN curl https://packages.osrfoundation.org/gazebo.gpg --output /usr/share/keyrings/pkgs-osrf-archive-keyring.gpg
# RUN echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/pkgs-osrf-archive-keyring.gpg] https://packages.osrfoundation.org/gazebo/ubuntu-stable $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/gazebo-stable.list > /dev/null
# RUN apt-get update
# RUN apt-get install -y ignition-fortress

# Transfer key
RUN mkdir -p /etc/apt/keyrings/
RUN wget -P /etc/apt/keyrings/ "https://huskyroboticsteam.github.io/ubuntu-repo/KEY.gpg"
RUN gpg --no-default-keyring --keyring ./temp-keyring.gpg --import /etc/apt/keyrings/KEY.gpg
RUN gpg --no-default-keyring --keyring ./temp-keyring.gpg --export --output /etc/apt/keyrings/KEY.gpg --yes
RUN rm temp-keyring.gpg*
RUN wget -P /etc/apt/sources.list.d "https://huskyroboticsteam.github.io/ubuntu-repo/husky_robotics.list"

RUN apt-get update \
    && apt-get install -y \
    build-essential \
    catch2 \
    frozen \
    git \
    gnupg2 \
    libavutil-dev \
    libboost-system-dev \
    libwebsocketpp-dev \
    libx264-dev \
    lsb-release \
    nlohmann-json3-dev \
    ublox-linux \
    unzip \
    && rm -rf /var/lib/apt/lists/*

ENV SHELL=/bin/bash

# ********************************************************
# * Anything else you want to do like clean up goes here *
# ********************************************************

# [Optional] Set the default user. Omit if you want to keep the default as root.
USER $USERNAME

# Update bashrc to use alias's, sourcing etc.
# Source ROS, and the workspace once it has been built
RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc \
    && echo '[ -f /home/ws/install/setup.bash ] && source /home/ws/install/setup.bash' >> ~/.bashrc
