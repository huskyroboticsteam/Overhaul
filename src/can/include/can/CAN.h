#pragma once

extern "C" {
#include <CAN26.h>
}

#include <can/msg/can_device.hpp>
#include <can/msg/can_packet.hpp>

#include <rclcpp/rclcpp.hpp>

/**
 * @namespace can
 * @brief Utilities for interacting with CAN devices.
 */
namespace can {

/**
 * @brief Creates the CAN Socket
 * 
 * @param can_name name of the CAN interface
 * @param logger optional, logger for errors
 * @param device optional, will enable reception if provided
 * 
 * @return a file descriptor, or -1 on failure
 */
int createCANSocket(std::string can_name, std::optional<rclcpp::Logger> logger, std::optional<CANDevice_t> device);

can::msg::CANDevice can2ros_device(CANDevice_t device);
CANDevice_t ros2can_device(can::msg::CANDevice device);

can::msg::CANPacket can2ros_packet(CANPacket_t packet);
CANPacket_t ros2can_packet(can::msg::CANPacket packet);

can::msg::CANPacket createMotorVelocityPacket(can::msg::CANDevice device, float vel);

} // namespace can