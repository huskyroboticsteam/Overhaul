#include "../include/mc/MissionControlProtocol.h"

#include <can/msg/can_device.hpp>
#include <rclcpp/rclcpp.hpp>

namespace mc {

MissionControlProtocol::MissionControlProtocol() : Node("mc_node") {
  this->can_pub_ = this->create_publisher<can::msg::CANPacket>(
    "can/tx", rclcpp::QoS(rclcpp::KeepAll())
  );

  this->drive_sub_ = this->create_subscription<mc::msg::DriveRequest>(
    "mc/drive_request", rclcpp::QoS(rclcpp::KeepAll()),
    std::bind(&MissionControlProtocol::handleDriveRequest, this, std::placeholders::_1)
  );

  this->joint_power_sub_ = this->create_subscription<mc::msg::JointPowerRequest>(
    "mc/joint_power_request", rclcpp::QoS(rclcpp::KeepAll()),
    std::bind(&MissionControlProtocol::handleJointPowerRequest, this, std::placeholders::_1)
  );
}

void MissionControlProtocol::handleDriveRequest(const mc::msg::DriveRequest::SharedPtr msg) const {
  geometry_msgs::msg::Twist dir = msg->dir;
  geometry_msgs::msg::Vector3 drive = dir.linear;
  geometry_msgs::msg::Vector3 orientation = dir.angular;

  RCLCPP_DEBUG(this->get_logger(), "%f, %f, %f (%f, %f, %f)", drive.x, drive.y, drive.z, orientation.x, orientation.y, orientation.z);
}

void MissionControlProtocol::handleJointPowerRequest(const mc::msg::JointPowerRequest::SharedPtr msg) const {
  std::string joint = msg->joint;
  double power = msg->power;

  can::msg::CANDevice device;
  // device.device_uuid = 

  can::msg::CANPacket packet;


  RCLCPP_INFO(this->get_logger(), "%s at %f", joint.c_str(), power);
}

} // namespace mc

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);

  rclcpp::executors::SingleThreadedExecutor executor;
  rclcpp::Node::SharedPtr node = std::make_shared<mc::MissionControlProtocol>();
  executor.add_node(node);

	std::thread thread = std::thread([&]() { executor.spin(); });

	while (rclcpp::ok()) {}

  rclcpp::shutdown();
  thread.join();

  return 0;
}
