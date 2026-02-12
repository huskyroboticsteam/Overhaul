#include <string>

#include <rclcpp/rclcpp.hpp>

class Rover : public rclcpp::Node {
  public:
    Rover() : Node("rover_node") {
      auto peripheral_desc = rcl_interfaces::msg::ParameterDescriptor{};
      peripheral_desc.description = "Specifies which peripheral is mounted, default none";
      this->declare_parameter("peripheral", "none", peripheral_desc);

      std::string peripheral = this->get_parameter("peripheral").as_string();
      RCLCPP_INFO(this->get_logger(), "Peripheral: %s", peripheral.c_str());
    }
  private:
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Rover>());
  rclcpp::shutdown();

  return 0;
}
