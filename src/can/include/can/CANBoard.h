#pragma once

#include "CAN.h"

namespace can { /*

constexpr auto jointMotorMap = frozen::make_unordered_map<frozen::string, CANDevice_t>(
  {
    {"armBase", CANDevice_t{CAN_UUID_BLDC_BASE, 1, 0, 0}}
  }
);

constexpr auto velocityMap = frozen::make_unordered_map<CANDeviceUUID_t, double>(
  {
    {CAN_UUID_BLDC_BASE, 10.0}
  }
);

this->timer_ = this->create_wall_timer(std::chrono::milliseconds(333), std::bind(Controller::periodicTask, this));

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

*/ } // namespace can