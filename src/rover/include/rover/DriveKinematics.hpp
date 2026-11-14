#pragma once
#include <cmath>
#include <initializer_list>
#include <stdexcept>

namespace rover::drive {
struct WheelTarget { double velocity; double steering_angle; };

// x/y is the wheel position in the rover frame (x forward, y left).
inline WheelTarget wheelTarget(double vx, double vy, double yaw_rate,
    double x, double y, double radius, bool fixed_wheel) {
  for (double value : {vx, vy, yaw_rate, x, y, radius}) {
    if (!std::isfinite(value)) { throw std::invalid_argument("Non-finite drive input/configuration"); }
  }
  if (radius <= 0) { throw std::invalid_argument("Wheel radius must be positive"); }
  const double wx = vx - yaw_rate * y;
  const double wy = vy + yaw_rate * x;
  if (!std::isfinite(wx) || !std::isfinite(wy)) {
    throw std::invalid_argument("Drive kinematics overflow");
  }
  if (fixed_wheel) {
    if (std::abs(vy) > 1e-6 || std::abs(yaw_rate) > 1e-6) {
      throw std::invalid_argument("Single fixed wheel supports linear.x only");
    }
    return {wx / radius, 0};
  }
  double speed = std::hypot(wx, wy) / radius;
  double angle = speed == 0 ? 0 : std::atan2(wy, wx);
  constexpr double pi = 3.14159265358979323846;
  // Equivalent steering solution within +/- pi/2 preserves signed reverse speed.
  if (angle > pi / 2) { angle -= pi; speed = -speed; }
  if (angle < -pi / 2) { angle += pi; speed = -speed; }
  return {speed, angle};
}
}  // namespace rover::drive
