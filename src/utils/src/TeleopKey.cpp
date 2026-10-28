#include "../include/utils/KeyboardReader.h"

#include <rclcpp/rclcpp.hpp> 

namespace utils {

class TeleopKey : public rclcpp::Node {
  public:
    TeleopKey() : Node("teleop_node"), input_(), dirty_(false) {
      this->timer_ = this->create_wall_timer(std::chrono::nanoseconds(0), std::bind(&TeleopKey::getKey, this));
    }

    ~TeleopKey() {
      RCLCPP_INFO(this->get_logger(), "Shutdown");
      this->input_.shutdown();
    }

  private:
    int getKey() {
      if (!rclcpp::ok()) { return 0; }

      try {
        this->input_.readOne(&this->c_);
        RCLCPP_INFO(this->get_logger(), "Key: %c", this->c_);

      } catch (const std::runtime_error &) {
        perror("read():");
        return -1;
      }

      switch(this->c_) {
        case 'w':
          break;
        default:
          break;
      }

      return 0;
    }

    KeyboardReader input_;
    rclcpp::TimerBase::SharedPtr timer_;

    bool dirty_;
    char c_;
};

} // namespace utils

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<utils::TeleopKey>());
  rclcpp::shutdown();
  return 0;
}