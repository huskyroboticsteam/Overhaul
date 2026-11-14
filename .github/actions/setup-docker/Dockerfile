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
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME

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
    ros-humble-ros-gz \
    && rm -rf /var/lib/apt/lists/*

ENV SHELL=/bin/bash

# ********************************************************
# * Anything else you want to do like clean up goes here *
# ********************************************************

# Update bashrc to use alias's, sourcing etc.
# Source ROS, and the workspace once it has been built
RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc \
    && echo '[ -f /home/ws/install/setup.bash ] && source /home/ws/install/setup.bash' >> ~/.bashrc

COPY run_tests.sh /run_tests.sh

ENTRYPOINT ["./run_tests.sh"]