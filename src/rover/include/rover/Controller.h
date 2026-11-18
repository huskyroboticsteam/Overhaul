#pragma once

#include <std_msgs/msg/float32.hpp>

#include <can/include/can/CAN.h>

#include <frozen/string.h>
#include <frozen/unordered_map.h>
#include <rclcpp/rclcpp.hpp>

constexpr auto jointMotorMap = frozen::make_unordered_map<frozen::string, CANDevice_t>(
  {
    {"armBase", CANDevice_t{CAN_UUID_BLDC_BASE, 1, 0, 0}}
  }
);

constexpr auto velocityMap = frozen::make_unordered_map<CANDeviceUUID_t, double>(
  {
    {CAN_UUID_BLDC_BASE, 10.0}
  }
);

class Controller : public rclcpp::Node {
public:
  Controller();

private:
  std::mutex mutex_;
  std::unordered_map<CANDeviceUUID_t, double> last_motor_power_;

  rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr subscription_;
  void handlePowerUpdate(const std_msgs::msg::Float32::SharedPtr msg, std::string joint);

  rclcpp::Publisher<can::msg::CANPacket>::SharedPtr can_pub_;

  rclcpp::TimerBase::SharedPtr timer_;
  void periodicTask();
};