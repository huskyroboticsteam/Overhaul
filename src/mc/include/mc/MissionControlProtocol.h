#pragma once

#include <can/msg/can_packet.hpp>
#include <mc/msg/drive_request.hpp>
#include <mc/msg/joint_power_request.hpp>

#include <rclcpp/rclcpp.hpp>

#include <frozen/string.h>
#include <frozen/unordered_map.h>

namespace mc {

class MissionControlProtocol : public rclcpp::Node {
  public:
    MissionControlProtocol();

    void handleDriveRequest(const mc::msg::DriveRequest::SharedPtr msg) const;
    void handleJointPowerRequest(const mc::msg::JointPowerRequest::SharedPtr msg) const;

  private:
    rclcpp::Publisher<can::msg::CANPacket>::SharedPtr can_pub_;

    rclcpp::Subscription<mc::msg::DriveRequest>::SharedPtr drive_sub_;
    rclcpp::Subscription<mc::msg::JointPowerRequest>::SharedPtr joint_power_sub_;
    // publishers
};

constexpr auto jointIDMap = frozen::make_unordered_map<frozen::string, uint16_t>(
  {
    {"shoulder", 0x35}
  }
);

} // namespace mc