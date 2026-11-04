#include <cmath>
#include <memory>
#include <string>
#include <functional>
#include <rclcpp/rclcpp.hpp>
#include "geometry_msgs/msg/twist.hpp"

using std::placeholders::_1;
using namespace rclcpp;
using namespace geometry_msgs;
using namespace msg;


class Drive : public rclcpp::Node {
  public:
    Drive() : Node("Drive_node") {
      RCLCPP_INFO(this->get_logger(), "Started drive node");

      dir_subscriber_ = this->create_subscription<geometry_msgs::msg::Twist>(
        "direction", 10, std::bind(&Drive::dir_callback, this, _1));

      for (const std::string &curr_el : DRIVE_UNITS) {
        Publisher<Twist>::SharedPtr ptr = 
          this->create_publisher<geometry_msgs::msg::Twist>(curr_el, 10);
        drive_unit_publishers.push_back(ptr);
      }
    }
  private:

    // Sim swerve drive targets
    const std::vector<std::string> DRIVE_UNITS{"/Direction/BackLeft",
                                              "/Direction/BackRight",
                                              "/Direction/FrontLeft",
                                              "/Direction/FrontRight"};

    void dir_callback(const geometry_msgs::msg::Twist desired_vec) const {

      RCLCPP_INFO(this->get_logger(), "x: %f, y: %f", desired_vec.linear.x, desired_vec.linear.y);

      // Desired rotation state
      double desired_rot = std::atan2(desired_vec.linear.y, desired_vec.linear.x);

      // Sim handling
      Twist::SharedPtr desired_swerve_state = std::make_shared<Twist>();
      desired_swerve_state->angular.x = desired_rot;
      desired_swerve_state->linear.x = 
        std::sqrt(std::pow(desired_vec.linear.x, 2) + 
        std::pow(desired_vec.linear.y, 2));

      RCLCPP_INFO(this->get_logger(), "mag, %f desired, %f", desired_swerve_state->linear.x, desired_rot);

      for (Publisher<Twist>::SharedPtr motor_publisher : drive_unit_publishers) {
        motor_publisher->publish(*desired_swerve_state);
      }
    }
    Subscription<Twist>::SharedPtr dir_subscriber_;
    std::vector<Publisher<Twist>::SharedPtr> drive_unit_publishers;

};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Drive>());
  rclcpp::shutdown();
  return 0;
}
