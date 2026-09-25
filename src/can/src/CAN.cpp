#include "../include/can/CAN.h"

#include <rclcpp/rclcpp.hpp>

class CAN : public rclcpp::Node {
  public:
    CAN() : Node("CAN_node") {
      RCLCPP_INFO(this->get_logger(), "CAN Node");
      initCAN();
    }

    void initCAN() {
      RCLCPP_INFO(this->get_logger(), "Initializing CAN");
    }
  private:
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CAN>());
  rclcpp::shutdown();
  return 0;
}
