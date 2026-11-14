#include "rover/MotorController.hpp"
#include "rover/DriveKinematics.hpp"
#include <iostream>

using namespace rover::drive;
#define CHECK(expr) do { if (!(expr)) { \
  std::cerr << "Failed at line " << __LINE__ << ": " << #expr << '\n'; return 1; \
} } while (false)

class ControlledBackend : public MotorBackend {
 public:
  bool is_ready = true;
  int stops = 0;
  int sends = 0;
  double output = 0;
  std::optional<Feedback> feedback;
  bool ready() const override { return is_ready; }
  void setVelocity(double value, double) override { output = value; ++sends; }
  void stop() override { output = 0; ++stops; }
  void requestFeedback(uint64_t, double) override {}
  std::optional<Feedback> takeFeedback() override {
    auto result = feedback; feedback.reset(); return result;
  }
  void tick(double) override {}
};

int main() {
  auto reverse = wheelTarget(-0.3, 0, 0, 0, 0, 0.15, true);
  CHECK(std::abs(reverse.velocity + 2) < 1e-9 && reverse.steering_angle == 0);
  auto swerve = wheelTarget(-0.3, 0, 0, 0, 0, 0.15, false);
  CHECK(std::abs(swerve.velocity + 2) < 1e-9 && std::abs(swerve.steering_angle) < 1e-9);
  auto turn = wheelTarget(0, 0, 1, 0.5, 0.5, 0.1, false);
  CHECK(std::abs(turn.velocity * std::cos(turn.steering_angle) + 5) < 1e-9);
  CHECK(std::abs(turn.velocity * std::sin(turn.steering_angle) - 5) < 1e-9);
  bool rejected = false;
  try { wheelTarget(0, 1, 0, 0, 0, 0.15, true); }
  catch (const std::invalid_argument &) { rejected = true; }
  CHECK(rejected);

  {
    FakeMotorBackend backend;
    MotorController motor(backend, {});
    CHECK(!motor.feedbackValid(0));
    CHECK(std::isnan(motor.state().measured_velocity));
    CHECK(motor.setVelocity(-2, 0, 0));
    CHECK(motor.state().result == Result::Pending);
    motor.tick(0.01);
    CHECK(motor.state().result == Result::Verified);
    CHECK(motor.state().measured_velocity == -2);
    motor.tick(0.51);
    CHECK(motor.state().result == Result::Expired);
    motor.tick(0.52);
    CHECK(motor.state().measured_velocity == 0);
    CHECK(motor.setVelocity(1, 0, 0.6));
    motor.tick(0.61);
    CHECK(motor.state().result == Result::Verified);
  }
  {
    ControlledBackend backend;
    MotorController motor(backend, {});
    CHECK(motor.setVelocity(2, 0, 0));
    const auto old_id = motor.state().command_id;
    CHECK(motor.setVelocity(-2, 0, 0.1));
    backend.feedback = Feedback{old_id, -2};
    motor.tick(0.11);
    CHECK(motor.state().result == Result::Pending);
    backend.feedback = Feedback{motor.state().command_id, -2};
    motor.tick(0.12);
    CHECK(motor.state().result == Result::Verified);
  }
  {
    ControlledBackend backend;
    MotorController motor(backend, {});
    CHECK(motor.setVelocity(2, 0, 0));
    CHECK(motor.setVelocity(2, 0, 0.4));
    CHECK(motor.state().command_id == 1);
    motor.tick(0.51);
    CHECK(motor.state().result == Result::Failed);
    CHECK(backend.output == 0);
    CHECK(!motor.setVelocity(2, 0, 0.6));  // Fault latches until restart.
  }
  {
    ControlledBackend backend;
    MotorController motor(backend, {});
    for (int i = 0; i < 7; ++i) {
      motor.setVelocity(i, 0, i * 0.1);
      motor.tick(i * 0.1);
    }
    CHECK(motor.state().result == Result::Failed);  // Changing targets cannot hide lost feedback.
  }
  {
    ControlledBackend backend;
    MotorController motor(backend, {});
    CHECK(motor.setVelocity(2, 0, 0));
    for (int i = 1; i <= 12; ++i) {
      const double time = i * 0.1;
      motor.setVelocity(2, 0, time);
      backend.feedback = Feedback{1, 0};
      motor.tick(time);
    }
    CHECK(motor.state().result == Result::Failed);
    CHECK(backend.output == 0);
  }
  for (double bad : {11.0, std::numeric_limits<double>::infinity(),
      std::numeric_limits<double>::quiet_NaN()}) {
    ControlledBackend backend;
    MotorController motor(backend, {});
    CHECK(!motor.setVelocity(bad, 0, 0));
    CHECK(motor.state().result == Result::Failed && backend.stops == 1);
  }
  {
    ControlledBackend backend;
    MotorController motor(backend, {});
    CHECK(!motor.setVelocity(1, 0.2, 0));
    CHECK(backend.output == 0);
  }
  {
    ControlledBackend backend;
    backend.is_ready = false;
    MotorController motor(backend, {});
    CHECK(!motor.setVelocity(2, 0, 0));
    backend.is_ready = true;
    motor.tick(0.1);
    CHECK(backend.output == 0);  // Discarded commands never execute on readiness.
    CHECK(motor.setVelocity(2, 0, 0.2));
  }
  std::cout << "Drive core tests passed\n";
  return 0;
}
