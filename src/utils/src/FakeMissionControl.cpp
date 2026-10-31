#include "../include/utils/KeyboardReader.h"

#include <mc/msg/mc_packet.hpp>
#include <rclcpp/rclcpp.hpp> 

namespace utils {

class FakeMissionControl : public rclcpp::Node {
  public:
    FakeMissionControl() : Node("fake_mc"), input_(), dirty_(false) {
      this->timer_ = this->create_wall_timer(std::chrono::nanoseconds(0), std::bind(&FakeMissionControl::getKey, this));

      this->pub_ = this->create_publisher<mc::msg::MCPacket>("/mc", rclcpp::QoS(rclcpp::KeepAll()));
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

      mc::msg::MCPacket packet;
      switch(this->c_) {
        case KEYCODE_UP:
          packet.type = "drive up";
          this->dirty_ = true;
          break;

        case 'w':
          packet.type = "drive";
          this->dirty_ = true;
          break;

        default:
          break;
      }

      if (this->dirty_) {
        this->pub_->publish(packet);
        this->dirty_ = false;
      }

      return 0;
    }

    KeyboardReader input_;
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<mc::msg::MCPacket>::SharedPtr pub_;

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