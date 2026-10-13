#include <string>

#include <rclcpp/rclcpp.hpp> 
#include "geometry_msgs/msg/twist.hpp"

using std::placeholders::_1;

class Rover : public rclcpp::Node {
  public:
    Rover() : Node("rover_node") {
      auto peripheral_desc = rcl_interfaces::msg::ParameterDescriptor{};
      peripheral_desc.description = "Specifies which peripheral is mounted, default none";
      this->declare_parameter("peripheral", "none", peripheral_desc);

      dir_subscriber_ = this->create_subscription<geometry_msgs::msg::Twist>(
        "direction", 10, std::bind(&Rover::dir_callback, this, _1));

      std::string peripheral = this->get_parameter("peripheral").as_string();
      RCLCPP_INFO(this->get_logger(), "Peripheral: %s", peripheral.c_str());
    }
  private:
    void dir_callback(const geometry_msgs::msg::Twist desired_vec) const {
      std::cout << "test" << std::endl;
    }
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr dir_subscriber_;
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Rover>());
  rclcpp::shutdown();
  return 0;
}
