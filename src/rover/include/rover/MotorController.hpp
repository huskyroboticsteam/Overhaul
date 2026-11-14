#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>

namespace rover::drive {

struct Feedback {
  uint64_t command_id;
  double velocity;
};

// Calls are serialized by the ROS executor. All times are steady-clock seconds.
class MotorBackend {
 public:
  virtual ~MotorBackend() = default;
  virtual bool ready() const = 0;
  virtual bool supportsSteering() const { return false; }
  virtual std::string fault() const { return {}; }
  virtual void setVelocity(double velocity, double steering_angle) = 0;
  virtual void stop() = 0;
  virtual void requestFeedback(uint64_t command_id, double now) = 0;
  virtual std::optional<Feedback> takeFeedback() = 0;
  virtual void tick(double now) = 0;
};

enum class Result : uint8_t { Idle, Pending, Verified, Failed, Expired };

struct MotorConfig {
  double max_velocity = 10.0;
  double command_timeout = 0.5;
  double feedback_timeout = 0.5;
  double settle_timeout = 1.0;
  double velocity_tolerance = 0.25;
  double update_period = 0.1;
};

struct MotorState {
  uint64_t command_id = 0;
  double requested_velocity = 0.0;
  double measured_velocity = std::numeric_limits<double>::quiet_NaN();
  double feedback_time = -1.0;
  Result result = Result::Idle;
  std::string fault;
};

class MotorController {
 public:
  MotorController(MotorBackend & backend, MotorConfig config)
      : backend_(backend), config_(config) {
    for (double value : {config.max_velocity, config.command_timeout,
        config.feedback_timeout, config.settle_timeout, config.velocity_tolerance,
        config.update_period}) {
      if (!std::isfinite(value) || value <= 0) {
        throw std::invalid_argument("Motor timeouts, limit and tolerance must be finite and positive");
      }
    }
    if (config.update_period >= config.command_timeout ||
        config.update_period >= config.feedback_timeout) {
      throw std::invalid_argument("Update period must be shorter than command/feedback timeouts");
    }
  }

  bool setVelocity(double velocity, double steering, double now) {
    if (latched_fault_) { return false; }
    if (!std::isfinite(velocity) || !std::isfinite(steering) ||
        std::abs(velocity) > config_.max_velocity ||
        (!backend_.supportsSteering() && std::abs(steering) > 1e-6)) {
      fail("Invalid command: check finite values, velocity limit and steering support");
      return false;
    }
    if (!backend_.fault().empty()) {
      fail(backend_.fault());
      return false;
    }
    if (!backend_.ready()) {
      state_.result = Result::Failed;
      state_.fault = "Motor backend is not ready; command discarded";
      return false;
    }
    last_command_ = now;
    // Repeated commands renew the lease without restarting the confirmation deadline.
    if (active_ && velocity == state_.requested_velocity && steering == steering_) {
      return true;
    }
    ++state_.command_id;
    state_.requested_velocity = velocity;
    steering_ = steering;
    state_.result = Result::Pending;
    state_.fault.clear();
    if (!active_) { started_ = now; }
    active_ = true;
    last_match_ = now;
    backend_.setVelocity(velocity, steering);
    last_update_ = now;
    backend_.requestFeedback(state_.command_id, now);
    return true;
  }

  void tick(double now) {
    backend_.tick(now);
    if (!backend_.fault().empty() && !latched_fault_) { fail(backend_.fault()); }
    if (auto sample = backend_.takeFeedback(); sample && std::isfinite(sample->velocity)) {
      state_.measured_velocity = sample->velocity;
      state_.feedback_time = now;
      if (active_ && sample->command_id == state_.command_id &&
          std::abs(sample->velocity - state_.requested_velocity) <= config_.velocity_tolerance) {
        state_.result = Result::Verified;
        last_match_ = now;
      }
    }
    if (active_ && now - last_command_ >= config_.command_timeout) {
      active_ = false;
      state_.result = Result::Expired;
      state_.fault = "Command expired; zero velocity requested";
      backend_.stop();
    }
    if (active_) {
      const double last_feedback = std::max(started_, state_.feedback_time);
      if (now - last_feedback >= config_.feedback_timeout) {
        fail("Velocity feedback timed out");
      } else if (now - last_match_ >= config_.settle_timeout) {
        fail("Measured velocity did not track the requested velocity");
      }
    }
    if (backend_.ready() && now - last_update_ >= config_.update_period) {
      if (active_) { backend_.setVelocity(state_.requested_velocity, steering_); }
      else { backend_.stop(); }
      backend_.requestFeedback(state_.command_id, now);
      last_update_ = now;
    }
  }

  const MotorState & state() const { return state_; }
  bool ready() const { return backend_.ready() && !latched_fault_; }
  bool feedbackValid(double now) const {
    return state_.feedback_time >= 0 && now - state_.feedback_time < config_.feedback_timeout;
  }

 private:
  void fail(const std::string & reason) {
    active_ = false;
    latched_fault_ = true;
    state_.result = Result::Failed;
    state_.fault = reason;
    backend_.stop();
  }
  MotorBackend & backend_;
  MotorConfig config_;
  MotorState state_;
  bool active_ = false;
  bool latched_fault_ = false;
  double steering_ = 0;
  double last_command_ = 0;
  double started_ = 0;
  double last_match_ = 0;
  double last_update_ = -1;
};

class FakeMotorBackend : public MotorBackend {
 public:
  bool ready() const override { return true; }
  bool supportsSteering() const override { return true; }
  void setVelocity(double velocity, double) override { velocity_ = velocity; }
  void stop() override { velocity_ = 0; }
  void requestFeedback(uint64_t command_id, double) override {
    feedback_ = Feedback{command_id, velocity_};
  }
  std::optional<Feedback> takeFeedback() override {
    auto result = feedback_;
    feedback_.reset();
    return result;
  }
  void tick(double) override {}
 private:
  double velocity_ = 0;
  std::optional<Feedback> feedback_;
};
}  // namespace rover::drive
