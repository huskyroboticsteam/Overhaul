#pragma once

#include <string>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/quaternion_stamped.hpp"
#include "sensor_msgs/msg/imu.hpp"

#include "unitree_lidar_sdk.h"

class UnitreeLidarSDKNode : public rclcpp::Node
{
public:
  explicit UnitreeLidarSDKNode(const rclcpp::NodeOptions &options = rclcpp::NodeOptions());
  ~UnitreeLidarSDKNode() override;

  void timer_callback();

private:
  // ROS
  rclcpp::Publisher<geometry_msgs::msg::QuaternionStamped>::SharedPtr pub_quat_;
  rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr pub_imu_;
  rclcpp::TimerBase::SharedPtr timer_;

  // Unitree Lidar Reader (SDK2)
  unilidar_sdk2::UnitreeLidarReader *lsdk_{nullptr};

  // Config params
  std::string port_;
  int baudrate_;
  int cloud_scan_num_;
  double range_max_;
  double range_min_;

  std::string imu_frame_;
  std::string imu_topic_;

  std::string quat_frame_;
  std::string quat_topic_;
};