#include "../include/utils/KeyboardReader.h"

#include <mc/msg/joint_power_request.hpp>

#include <rclcpp/rclcpp.hpp> 

namespace utils {

class FakeMissionControl : public rclcpp::Node {
  public:
    FakeMissionControl() : Node("fake_mc"), input_(), dirty_(false) {
      this->timer_ = this->create_wall_timer(std::chrono::nanoseconds(0), std::bind(&FakeMissionControl::getKey, this));

      this->joint_power_pub_ = this->create_publisher<mc::msg::JointPowerRequest>(
        "mc/joint_power_request", rclcpp::QoS(rclcpp::KeepAll())
      );
    }

    ~FakeMissionControl() {
      RCLCPP_INFO(this->get_logger(), "Shutdown");
      this->input_.shutdown();
    }

  private:
    int getKey() {
      try {
        this->input_.readOne(&this->c_);
        RCLCPP_INFO(this->get_logger(), "Key: %d", this->c_);

      } catch (const std::runtime_error &) {
        perror("read():");
        return -1;
      }

      mc::msg::JointPowerRequest joint_power_packet;

      switch(this->c_) {
        case KEYCODE_UP:
          // packet.type = "drive up";
          // this->dirty_ = true;
          break;

        case 'w':
          joint_power_packet.joint = "shoulder";
          joint_power_packet.power = 1.0;
          this->joint_power_pub_->publish(joint_power_packet);
          break;

        case 's':
          joint_power_packet.joint = "shoulder";
          joint_power_packet.power = -1.0;
          this->joint_power_pub_->publish(joint_power_packet);
          break;

        default:
          break;
      }

      return 0;
    }

    KeyboardReader input_;
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<mc::msg::JointPowerRequest>::SharedPtr joint_power_pub_;

    bool dirty_;
    char c_;
};

} // namespace utils

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<utils::FakeMissionControl>());
  rclcpp::shutdown();
  return 0;
}