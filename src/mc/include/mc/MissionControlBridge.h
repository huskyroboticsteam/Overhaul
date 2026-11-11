#pragma once

#include <mc/msg/drive_request.hpp>
#include <mc/msg/joint_power_request.hpp>
#include <std_msgs/msg/float32.hpp>

#include <frozen/string.h>
#include <frozen/unordered_map.h>
#include <rclcpp/rclcpp.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

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

  void handleDriveRequest(const mc::msg::DriveRequest::SharedPtr msg) const;
  void handleJointPowerRequest(const mc::msg::JointPowerRequest::SharedPtr msg) const;

private:
  // std::string server_name_;
  uint16_t port_;
  websocketpp::server<websocketpp::config::asio> server_;
  // publishers
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr drive_pub_;
  rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr elbow_pub_;

  rclcpp::TimerBase::SharedPtr timer_;

  void onOpen(websocketpp::connection_hdl hdl);
  void onMessage(websocketpp::connection_hdl hdl, std::shared_ptr<websocketpp::config::core::message_type> message);
};

constexpr auto jointMotorMap = frozen::make_unordered_map<frozen::string, frozen::string>({
    { "base", "base" },
    { "shoulder", "shoulder" },
    { "elbow", "elbow" },
    { "forearm", "forearm" },
    { "hand", "hand" },
});

}  // namespace mc