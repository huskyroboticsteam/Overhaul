#include "../include/rover/Controller.h"

Controller::Controller() : Node("controller"), last_motor_power_() {
  this->subscription_ = this->create_subscription<std_msgs::msg::Float32>(
    "/mc/armBase_power", rclcpp::QoS(rclcpp::KeepAll()), std::bind(
      Controller::handlePowerUpdate, this, std::placeholders::_1, "armBase"
    )
  );

  this->can_pub_ = this->create_publisher<can::msg::CANPacket>(
    "/can/tx", rclcpp::QoS(rclcpp::KeepAll()));

  this->timer_ = this->create_wall_timer(std::chrono::milliseconds(333), std::bind(Controller::periodicTask, this));
}

void Controller::handlePowerUpdate(const std_msgs::msg::Float32::SharedPtr msg, std::string joint) {
  if (auto it = jointMotorMap.find(joint); it != jointMotorMap.end()) {
    CANDevice_t device = it->second;
    double velocity = velocityMap.at(device.deviceUUID) * msg->data;

    std::lock_guard lock(this->mutex_);
    this->last_motor_power_.try_emplace(device.deviceUUID, velocity);
  }
}

void Controller::periodicTask() {
  std::lock_guard lock(this->mutex_);
  for (const auto pair : this->last_motor_power_) {
    const uint16_t motor = pair.first;
    const double vel = pair.second;

    if (vel != 0.0) {
      CANDevice_t jetson = CANDevice_t{ 0, 0, 0, 0x1 };
      CANPacket_t packet = CANMotorPacket_BLDC_SetInputVelocity(jetson, CANDevice_t{motor, 0, 1, 0}, vel, 0.0f);

      this->can_pub_->publish(can::can2ros_packet(packet));
    }
  }
}

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Controller>());
  rclcpp::shutdown();
  return 0;
}