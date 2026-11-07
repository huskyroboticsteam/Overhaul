#include "../include/mc/MissionControlBridge.h"

#include <can/msg/can_device.hpp>
#include <rclcpp/rclcpp.hpp>

namespace mc {

MissionControlBridge::MissionControlBridge(const std::string &server_name, uint16_t port)
  : Node("mc_node"), server_name_(server_name), port_(port), server_() {
  // disable websocket logging
  this->server_.set_access_channels(websocketpp::log::alevel::none);
  this->server_.set_error_channels(websocketpp::log::elevel::none);
	this->server_.set_reuse_addr(true);
	this->server_.init_asio();

  this->server_.set_open_handler([&](websocketpp::connection_hdl hdl){ this->onOpen(hdl); });

  this->elbow_pub_ = this->create_publisher<std_msgs::msg::Float32>(
    "motor/elbow/power", rclcpp::QoS(rclcpp::KeepAll())
  );

}

void MissionControlBridge::handleDriveRequest(const mc::msg::DriveRequest::SharedPtr msg) const {
  geometry_msgs::msg::Twist dir = msg->dir;
  geometry_msgs::msg::Vector3 drive = dir.linear;
  geometry_msgs::msg::Vector3 orientation = dir.angular;

  RCLCPP_DEBUG(this->get_logger(), "%f, %f, %f (%f, %f, %f)", drive.x, drive.y, drive.z, orientation.x, orientation.y, orientation.z);
}

void MissionControlBridge::handleJointPowerRequest(const mc::msg::JointPowerRequest::SharedPtr msg) const {
  std::string joint = msg->joint;
  float power = msg->power;

  RCLCPP_INFO(this->get_logger(), "%s at %f", joint.c_str(), power);

  can::msg::CANDevice device;
  auto motor = mc::jointMotorMap.find(frozen::string(joint));
  if (motor == mc::jointMotorMap.end()) {
    // go to other part
    RCLCPP_ERROR(this->get_logger(), "no defined control for joint %s!", joint.c_str());
    return;
  } else if (motor->second == "elbow") {
    this->elbow_pub_->publish(std_msgs::build<std_msgs::msg::Float32>().data(power));

    // device = mc::motorDeviceMap.find(motor->second)->second;
    // float vel = power * 10.0f;

    // can::msg::CANPacket packet = can::createMotorVelocityPacket(device, vel);
    // this->can_pub_->publish(packet);
  }
}

void MissionControlBridge::onOpen(websocketpp::connection_hdl hdl) {
  auto conn = this->server_.get_con_from_hdl(hdl);
  std::string client = conn->get_remote_endpoint();
  std::string path = conn->get_resource();

  RCLCPP_INFO(this->get_logger(), "Server=%s, Endpoint=%s : Connection opened from %s",
    this->server_name_.c_str(), path.c_str(), client.c_str());
}

} // namespace mc

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);

  rclcpp::executors::SingleThreadedExecutor executor;
  rclcpp::Node::SharedPtr node = std::make_shared<mc::MissionControlBridge>();
  executor.add_node(node);

	std::thread thread = std::thread([&]() { executor.spin(); });

	while (rclcpp::ok()) {}

  rclcpp::shutdown();
  thread.join();

  return 0;
}
