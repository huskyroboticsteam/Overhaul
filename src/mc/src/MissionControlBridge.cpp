#include "../include/mc/MissionControlBridge.h"

#include <can/msg/can_device.hpp>
#include <rclcpp/rclcpp.hpp>

namespace mc {

MissionControlBridge::MissionControlBridge() : Node("mc_node"), server_() {
  // declare params
  // The name of this server. This is used only for logging purposes.
  // this->declare_parameter<std::string>("server_name", "mc_bridge");
  // The local port to bind to. This is not validated until start() is called.
  this->declare_parameter<uint16_t>("port", 3001);

  // this->server_name_ = this->get_parameter("server_name").as_string();
  this->port_ = this->get_parameter("port").get_value<uint16_t>();


  // disable websocket logging
  this->server_.set_access_channels(websocketpp::log::alevel::none);
  this->server_.set_error_channels(websocketpp::log::elevel::none);
	this->server_.set_reuse_addr(true);
	this->server_.init_asio();

  this->server_.set_open_handler([&](websocketpp::connection_hdl hdl){ this->onOpen(hdl); });
	this->server_.set_message_handler(
		[&](websocketpp::connection_hdl hdl, message_t msg) { this->onMessage(hdl, msg); });

  this->elbow_pub_ = this->create_publisher<std_msgs::msg::Float32>(
    "motor/elbow/power", rclcpp::QoS(rclcpp::KeepAll())
  );
}

MissionControlBridge::~MissionControlBridge() {
  RCLCPP_INFO(this->get_logger(), "shutting down...");
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

void MissionControlBridge::tryConnect() {
	try {
		this->server_.listen(this->port_);
    // std::string str = this->server_.get_local_endpoint().address().to_string().c_str();
    // std::string str = this->server_.get_connection()->get_raw_socket().
    // RCLCPP_INFO(this->get_logger(), str.c_str());
		this->server_.start_accept();
		this->server_.run();
	} catch (const websocketpp::exception& e) {
    RCLCPP_ERROR(this->get_logger(), "An error occurred while starting: %s", e.what());
	}
}

void MissionControlBridge::onOpen(websocketpp::connection_hdl hdl) {
  auto conn = this->server_.get_con_from_hdl(hdl);
  std::string client = conn->get_remote_endpoint();
  std::string path = conn->get_resource();

  RCLCPP_INFO(this->get_logger(), "Endpoint=%s : Connection opened from %s", path.c_str(), client.c_str());
}

void MissionControlBridge::onMessage(websocketpp::connection_hdl hdl, std::shared_ptr<websocketpp::config::core::message_type> message) {
  auto conn = this->server_.get_con_from_hdl(hdl);
  std::string path = conn->get_resource();
  
  std::string str = message->get_payload();

  RCLCPP_INFO(this->get_logger(), "Message on %s: %s", path.c_str(), str.c_str());
}

} // namespace mc

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);

  rclcpp::executors::SingleThreadedExecutor executor;
  auto node = std::make_shared<mc::MissionControlBridge>();
  executor.add_node(node);

	std::thread thread = std::thread([node]() {
    while (rclcpp::ok()) {
      node->tryConnect();
    }
  });

	while (rclcpp::ok()) {
    executor.spin();
  }

  rclcpp::shutdown();
  // thread.join();

  return 0;
}
