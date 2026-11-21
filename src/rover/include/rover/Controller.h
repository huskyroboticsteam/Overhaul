#pragma once

#include <std_msgs/msg/float32.hpp>

#include <frozen/string.h>
#include <frozen/unordered_map.h>
#include <frozen/unordered_set.h>
#include <rclcpp/rclcpp.hpp>

// constexpr auto jointMotorMap = frozen::make_unordered_set<frozen::string>({
//   "armBase",
//   "shoulder",
//   "elbow",
//   "forearm",
//   "hand"
// });

// constexpr auto jointMotorMap = frozen::make_unordered_map<frozen::string, frozen::string>({
//   {"armBase", "armBase"},
//   {"shoulder", "shoulder"},
//   {"elbow", "elbow"},
//   {"forearm", "forearm"},
//   {"hand", "hand"}
// });

const auto jointMotorMap = std::unordered_map<std::string, std::string>({
  {"armBase", "armBase"},
  {"shoulder", "shoulder"},
  {"elbow", "elbow"},
  {"forearm", "forearm"},
  {"hand", "hand"}
});

class Controller : public rclcpp::Node {
public:
  Controller();

private:
  std::unordered_set<rclcpp::SubscriptionBase::SharedPtr> joint_subs_;
  void createPowerSubscriber(std::string joint);

  std::unordered_map<frozen::string, rclcpp::PublisherBase::SharedPtr> motor_pub_map_;
  void createMotorPowerPublisher(std::string motor);

  void handlePowerUpdate(const std_msgs::msg::Float32::SharedPtr msg, std::string joint) const;
};