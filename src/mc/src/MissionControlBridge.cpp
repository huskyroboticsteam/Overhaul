#include "../include/mc/MissionControlBridge.h"

#include <can/msg/can_device.hpp>

#include <rclcpp/rclcpp.hpp>
#include <utils/json.h>

namespace mc {

MissionControlBridge::MissionControlBridge() : Node("mc_node"), server_() {
  // declare params

  // The local port to bind to. This is not validated until start() is called.
  this->declare_parameter<uint16_t>("port", 3001);

  // this->server_name_ = this->get_parameter("server_name").as_string();
  this->port_ = this->get_parameter("port").get_value<uint16_t>();

  // disable websocket logging
  this->server_.set_access_channels(websocketpp::log::alevel::none);
  this->server_.set_error_channels(websocketpp::log::elevel::none);
  this->server_.set_reuse_addr(true);
  this->server_.init_asio();

  this->server_.set_open_handler([&](websocketpp::connection_hdl hdl) { this->onOpen(hdl); });
  this->server_.set_message_handler([&](websocketpp::connection_hdl hdl, message_t msg) { this->onMessage(hdl, msg); });

  // publishers
  this->joint_pub_map_.try_emplace("armBase",
    this->create_publisher<std_msgs::msg::Float32>("joint/armBase/power", rclcpp::QoS(rclcpp::KeepAll())));
  this->joint_pub_map_.try_emplace("shoulder",
    this->create_publisher<std_msgs::msg::Float32>("joint/shoulder/power", rclcpp::QoS(rclcpp::KeepAll())));
  this->joint_pub_map_.try_emplace("elbow",
    this->create_publisher<std_msgs::msg::Float32>("joint/elbow/power", rclcpp::QoS(rclcpp::KeepAll())));
  this->joint_pub_map_.try_emplace("forearm",
    this->create_publisher<std_msgs::msg::Float32>("joint/forearm/power", rclcpp::QoS(rclcpp::KeepAll())));
  
  // define protocols
  this->addProtocol("jointPowerRequest",
    std::bind(&MissionControlBridge::handleJointPowerRequest, this, std::placeholders::_1),
    validateJointPowerRequest);
}

MissionControlBridge::~MissionControlBridge() {
  RCLCPP_INFO(this->get_logger(), "shutting down...");
}

void MissionControlBridge::tryConnect() {
  try {
    this->server_.listen(this->port_);
    this->server_.start_accept();
    this->server_.run();
  } catch (const websocketpp::exception& e) {
    RCLCPP_DEBUG(this->get_logger(), "An error occurred while starting: %s", e.what());
  }
}

void MissionControlBridge::addProtocol(frozen::string type, const std::function<void(const nlohmann::json&)> handler, const std::function<bool(const nlohmann::json&)> validator) {
  this->handlerMap.try_emplace(type, handler);
  this->validatorMap.try_emplace(type, validator);
}

void MissionControlBridge::onOpen(websocketpp::connection_hdl hdl) {
  auto conn = this->server_.get_con_from_hdl(hdl);
  std::string client = conn->get_remote_endpoint();
  std::string path = conn->get_resource();

  RCLCPP_INFO(this->get_logger(), "Endpoint=%s : Connection opened from %s", path.c_str(), client.c_str());
}

void MissionControlBridge::onMessage(websocketpp::connection_hdl hdl, message_t message) {
  auto conn = this->server_.get_con_from_hdl(hdl);
  std::string path = conn->get_resource();

  std::string str = message->get_payload();
  nlohmann::json obj = nlohmann::json::parse(str);

  if (!utils::hasKey(obj, "type")) {
    RCLCPP_WARN(this->get_logger(), "Malformed message without type: %s", str.c_str());
    return;
  }

  std::string type = obj.at("type");
  auto vit = this->validatorMap.find(frozen::string(type));
  if (vit == this->validatorMap.end()) {
    RCLCPP_WARN(this->get_logger(), "Unrecognized message type: %s", type.c_str());
    return;
  }

  if (vit->second(obj)) {
    RCLCPP_INFO(this->get_logger(), "MC->R: %s", str.c_str());
    this->handlerMap.at(frozen::string(type))(obj);
  } else {
    RCLCPP_WARN(this->get_logger(), "Invalid message received of type %s: %s", type.c_str(), str.c_str());
  }
}

static bool validateJointPowerRequest(const nlohmann::json& j) {
  return utils::validateKey(j, "joint", nlohmann::json::value_t::string) &&
         utils::validateRange(j, "power", -1, 1);
}
void MissionControlBridge::handleJointPowerRequest(const nlohmann::json& j) const {
  std::string joint = j.at("joint");

  auto it = this->joint_pub_map_.find(frozen::string(joint));
  if (it != this->joint_pub_map_.end()) {
    double power = j.at("power");
    auto msg = std_msgs::build<std_msgs::msg::Float32>().data(power);

    auto pub = std::dynamic_pointer_cast<rclcpp::Publisher<std_msgs::msg::Float32>>(it->second);
    pub->publish(msg);
  } else {
    RCLCPP_WARN(this->get_logger(), "No publisher for joint: %s", joint.c_str());
  }
}

}  // namespace mc

int main(int argc, char** argv) {
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
  thread.join();

  return 0;
}
