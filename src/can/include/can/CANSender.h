#pragma once

#include "./CAN.h"

#include <linux/can/raw.h>

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
    ~CANSender() override;

  private:
    void sendCANPacket(const can::msg::CANPacket::SharedPtr msg) const;
    bool sendCANFrame(const can_frame& frame) const;

    rclcpp::Subscription<can::msg::CANPacket>::SharedPtr subscription_;
    int fd_ = -1;
};

} // namespace can
