#include "rover/DriveKinematics.hpp"
#include <drive_interfaces/msg/wheel_command.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <rclcpp/rclcpp.hpp>
#include <memory>
#include <string>
#include <vector>

class Drive : public rclcpp::Node {
 public:
  Drive() : Node("drive") {
    const auto names = declare_parameter<std::vector<std::string>>("wheels", {"FrontLeft"});
    const auto xs = declare_parameter<std::vector<double>>("wheel_x", {0.0});
    const auto ys = declare_parameter<std::vector<double>>("wheel_y", {0.0});
    radius_ = declare_parameter("wheel_radius", 0.15);
    fixed_ = declare_parameter("fixed_wheel", true);
    max_velocity_ = declare_parameter("max_velocity", 10.0);
    if (names.empty() || names.size() != xs.size() || names.size() != ys.size() ||
        !std::isfinite(max_velocity_) || max_velocity_ <= 0) {
      throw std::invalid_argument("Invalid wheel arrays or maximum velocity");
    }
    for (size_t i = 0; i < names.size(); ++i) {
      rover::drive::wheelTarget(0, 0, 0, xs[i], ys[i], radius_, fixed_);
      wheels_.push_back({xs[i], ys[i], create_publisher<drive_interfaces::msg::WheelCommand>(
          "/Drive/" + names[i], rclcpp::QoS(1))});
    }
    subscription_ = create_subscription<geometry_msgs::msg::Twist>("/DriveRequest", rclcpp::QoS(1),
        [this](const geometry_msgs::msg::Twist & msg) { command(msg); });
  }

 private:
  void command(const geometry_msgs::msg::Twist & msg) {
    try {
      if (!std::isfinite(msg.linear.z) || !std::isfinite(msg.angular.x) ||
          !std::isfinite(msg.angular.y) || msg.linear.z != 0 || msg.angular.x != 0 || msg.angular.y != 0) {
        throw std::invalid_argument("DriveRequest supports planar motion only");
      }
      std::vector<rover::drive::WheelTarget> targets;
      for (const auto & wheel : wheels_) {
        auto target = rover::drive::wheelTarget(msg.linear.x, msg.linear.y, msg.angular.z,
            wheel.x, wheel.y, radius_, fixed_);
        if (!std::isfinite(target.velocity) || std::abs(target.velocity) > max_velocity_) {
          throw std::invalid_argument("DriveRequest exceeds wheel velocity limit");
        }
        targets.push_back(target);
      }
      for (size_t i = 0; i < targets.size(); ++i) {
        drive_interfaces::msg::WheelCommand output;
        output.velocity = targets[i].velocity;
        output.steering_angle = targets[i].steering_angle;
        wheels_[i].publisher->publish(output);
      }
    } catch (const std::invalid_argument & error) {
      RCLCPP_WARN(get_logger(), "Rejected DriveRequest: %s", error.what());
      for (const auto & wheel : wheels_) {
        wheel.publisher->publish(drive_interfaces::msg::WheelCommand{});
      }
    }
    // No timer replays commands: loss of /DriveRequest must expire the motor lease.
  }
  struct Wheel {
    double x, y;
    rclcpp::Publisher<drive_interfaces::msg::WheelCommand>::SharedPtr publisher;
  };
  double radius_, max_velocity_;
  bool fixed_;
  std::vector<Wheel> wheels_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr subscription_;
};

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  try { rclcpp::spin(std::make_shared<Drive>()); }
  catch (const std::exception & error) {
    RCLCPP_ERROR(rclcpp::get_logger("drive"), "%s", error.what());
    rclcpp::shutdown();
    return 1;
  }
  rclcpp::shutdown();
  return 0;
}
