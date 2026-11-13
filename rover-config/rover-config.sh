#!/usr/bin/env bash
sudo cp ./50-usb.rules /etc/udev/rules.d/

# Reloads udev rules instead of having to reboot
sudo udevadm control --reload-rules