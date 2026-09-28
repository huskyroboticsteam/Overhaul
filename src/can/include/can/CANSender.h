#pragma once

#include "./CAN.h"

/**
 * @namespace can
 * @brief Utilities for interacting with CAN devices.
 */
namespace can {

/**
 * @brief Node used to send CAN to the network.
 */
class CANSender : public rclcpp::Node {
  public:
    CANSender();

  private:
    void sendCANPacket(const can::msg::CANPacket::SharedPtr msg);
    bool sendCANFrame(const canfd_frame& frame);

    rclcpp::Subscription<can::msg::CANPacket>::SharedPtr subscription_;
    int fd_;
};

} // namespace can