#pragma once

extern "C" {
#include <CAN26.h>
}

#include <rclcpp/rclcpp.hpp>
#include <can/msg/can_packet.hpp>

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

} // namespace can