#pragma once

extern "C" {
#include <Packets/Motor.h>
#include <Packets/DecodeMotor.h>
}
#include <cmath>
#include <stdexcept>

namespace can::motor {
constexpr double TWO_PI = 6.28318530717958647692;

inline double motorRevsPerSecond(double wheel_rad_s, double gear_ratio, int direction) {
  if (!std::isfinite(wheel_rad_s) || !std::isfinite(gear_ratio) || gear_ratio <= 0 ||
      (direction != -1 && direction != 1)) {
    throw std::invalid_argument("Invalid motor velocity conversion");
  }
  return wheel_rad_s * gear_ratio * direction / TWO_PI;
}

inline CANPacket_t velocityPacket(CANDevice_t sender, CANDevice_t motor,
    double wheel_rad_s, double gear_ratio, int direction, bool request_ack = true) {
  const float velocity = static_cast<float>(motorRevsPerSecond(wheel_rad_s, gear_ratio, direction));
  if (!std::isfinite(velocity)) { throw std::invalid_argument("Motor velocity exceeds float32 range"); }
  auto packet = CANMotorPacket_BLDC_SetInputVelocity(sender, motor, velocity, 0.0f);
  if (request_ack) { packet.command = CAN_ACK(packet.command); }
  return packet;
}

inline double wheelVelocity(const CANPacket_t & packet, double gear_ratio, int direction) {
  if (packet.command != CAN_COMMAND_ID__BLDC_ENCODER_ESTIMATE || packet.contentsLength != 6) {
    throw std::invalid_argument("Invalid encoder response");
  }
  // Validate conversion parameters before decoding the compressed bfloat24 velocity.
  motorRevsPerSecond(0, gear_ratio, direction);
  return CANMotorPacket_BLDC_EncoderEstimates_Decode(&packet).velocity * TWO_PI /
         gear_ratio * direction;
}
}  // namespace can::motor
