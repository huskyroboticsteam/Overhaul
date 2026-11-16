#pragma once

#include <std_msgs/msg/float32.hpp>

#include <frozen/string.h>
#include <nlohmann/json.hpp>
#include <rclcpp/rclcpp.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <unordered_map>

namespace mc {

typedef std::shared_ptr<websocketpp::config::core::message_type> message_t;

/**
 * @brief The Mission Control Bridge, running on driver station laptop
 *
 */
class MissionControlBridge : public rclcpp::Node {
public:
  MissionControlBridge();
  ~MissionControlBridge();

  // delete assignment
  MissionControlBridge& operator=(const MissionControlBridge&) = delete;

  void tryConnect();
  
  void addProtocol(frozen::string path, const std::function<void(const nlohmann::json&)> handler, const std::function<bool(const nlohmann::json&)> validator);

  // void handleDriveRequest(const mc::msg::DriveRequest::SharedPtr msg) const;
  // void handleJointPowerRequest(const mc::msg::JointPowerRequest::SharedPtr msg) const;

private:
  uint16_t port_;
  websocketpp::server<websocketpp::config::asio> server_;

  std::unordered_map<frozen::string, std::function<void(const nlohmann::json&)>> handlerMap;
  std::unordered_map<frozen::string, std::function<bool(const nlohmann::json&)>> validatorMap;

  // publisher map
  std::unordered_map<frozen::string, rclcpp::PublisherBase::SharedPtr> joint_pub_map_;

  void onOpen(websocketpp::connection_hdl hdl);
  void onMessage(websocketpp::connection_hdl hdl, message_t message);

  void handleJointPowerRequest(const nlohmann::json&) const;
};

}  // namespace mc