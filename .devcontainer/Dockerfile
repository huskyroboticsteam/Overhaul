FROM ros:humble
ARG USERNAME=user
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Delete user if it exists in container (e.g Ubuntu Noble: ubuntu)
RUN if id -u $USER_UID ; then userdel `id -un $USER_UID` ; fi

# Create the user
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    #
    # [Optional] Add sudo support. Omit if you don't need to install software after connecting.
    && apt-get update \
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME
RUN apt-get update && apt-get upgrade -y

# Install dependencies and tools
RUN apt-get install -y python3-pip
RUN apt-get install -y vim
RUN apt-get install -y vim
ENV SHELL /bin/bash

# ********************************************************
# * Anything else you want to do like clean up goes here *
# ********************************************************

# Install MAVROS packages
RUN apt-get install -y ros-humble-mavros ros-humble-mavros-extras

# Install GeographicLib datasets (MAVROS requirement)
RUN wget https://raw.githubusercontent.com/mavlink/mavros/ros2/mavros/scripts/install_geographiclib_datasets.sh \
    && chmod +x install_geographiclib_datasets.sh \
    && ./install_geographiclib_datasets.sh \
    && rm install_geographiclib_datasets.sh

# [Optional] Set the default user. Omit if you want to keep the default as root.
USER $USERNAME

# Update bashrc to use alias's, sourcing etc.
RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc 

CMD ["/bin/bash"]