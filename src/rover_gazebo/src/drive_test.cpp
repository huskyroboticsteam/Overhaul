#include <chrono>

#include <rclcpp/rclcpp.hpp>
#include "geometry_msgs/msg/twist.hpp"

using namespace std::chrono_literals;

// Drives the rover in a circle to show that commands reach the simulation
class DriveTest : public rclcpp::Node {
  public:
    DriveTest() : Node("drive_test") {
      this->declare_parameter("linear_speed", 0.5);
      this->declare_parameter("angular_speed", 0.3);

      cmd_.linear.x = this->get_parameter("linear_speed").as_double();
      cmd_.angular.z = this->get_parameter("angular_speed").as_double();

      cmd_publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);
      timer_ = this->create_wall_timer(100ms, std::bind(&DriveTest::timer_callback, this));
    }
  private:
    void timer_callback() const {
      cmd_publisher_->publish(cmd_);
    }
    geometry_msgs::msg::Twist cmd_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DriveTest>());
  rclcpp::shutdown();
  return 0;
}
