#include "../include/rover/Controller.h"

Controller::Controller() : Node("controller") {
  // Move to some sort of constants file
  const std::array<std::string, 4> joints[] = {"armBase", "shoulder", "elbow", "forearm"};
  std::for_each(joints->begin(), joints->end(),
    [&](auto joint){ createPowerSubscriber(joint); }
  );

  // Create motor power publishers
  std::for_each(jointMotorMap.begin(), jointMotorMap.end(),
    [&](std::pair<std::string, std::string> pair){
      createMotorPowerPublisher(pair.second);
    }
  );

  // for (const auto &elem : this->motor_pub_map_) {
  //   RCLCPP_INFO(this->get_logger(), "%s", elem.first.data());
  // }
}

void Controller::createPowerSubscriber(std::string joint) {
  // Need to cast to function to make ROS happy...
  std::function<void(std_msgs::msg::Float32::SharedPtr)> callback = std::bind(
    &Controller::handlePowerUpdate, this, std::placeholders::_1, joint
  );

  this->joint_subs_.emplace(this->create_subscription<std_msgs::msg::Float32>(
    "/joint/" + joint + "/power", rclcpp::QoS(rclcpp::KeepAll()), callback
  ));
}

void Controller::createMotorPowerPublisher(std::string motor) {
  // RCLCPP_INFO(this->get_logger(), "%s", motor.c_str());
  auto result = this->motor_pub_map_.try_emplace(frozen::string(motor),
    this->create_publisher<std_msgs::msg::Float32>(
      "/board/" + motor + "/power", rclcpp::QoS(rclcpp::KeepAll())
    )
  );

  if (!result.second) {
    RCLCPP_WARN(this->get_logger(), "Could not create publisher for motor %s!", motor.c_str());
  }
}

void Controller::handlePowerUpdate(const std_msgs::msg::Float32::SharedPtr msg, std::string joint) const {
  // RCLCPP_INFO(this->get_logger(), "%s @ %f", joint.c_str(), msg->data);
  if (auto it = jointMotorMap.find(joint); it != jointMotorMap.end()) {
    std::string motor = it->second;
    RCLCPP_INFO(this->get_logger(), "motor %s", motor.c_str());

    auto pub_it = this->motor_pub_map_.find(frozen::string(motor));
    if (pub_it != this->motor_pub_map_.end()) {
      auto pub = std::dynamic_pointer_cast<rclcpp::Publisher<std_msgs::msg::Float32>>(pub_it->second);
      pub->publish(*msg);
    } else {
      RCLCPP_WARN(this->get_logger(), "No publisher for motor %s!", motor.c_str());
    }
  } else {
    RCLCPP_WARN(this->get_logger(), "No defined control for joint %s!", joint.c_str());
  }
}

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Controller>());
  rclcpp::shutdown();
  return 0;
}