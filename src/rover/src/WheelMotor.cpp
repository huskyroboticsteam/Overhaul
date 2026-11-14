#include "rover/CanMotorBackend.hpp"
#include "rover/MotorController.hpp"
#include <drive_interfaces/msg/motor_status.hpp>
#include <drive_interfaces/msg/wheel_command.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <std_msgs/msg/float64.hpp>
#include <chrono>
#include <memory>

namespace rover::drive {
// Adapter for the existing /Direction/{wheel} simulator output convention.
// A bridge must supply actual wheel rad/s on /Direction/{wheel}/velocity.
class SimMotorBackend : public MotorBackend {
 public:
  SimMotorBackend(rclcpp::Node & node, const std::string & wheel, double radius)
      : radius_(radius) {
    tx_ = node.create_publisher<geometry_msgs::msg::Twist>("/Direction/" + wheel, 1);
    rx_ = node.create_subscription<std_msgs::msg::Float64>(
        "/Direction/" + wheel + "/velocity", 1,
        [this](const std_msgs::msg::Float64 & msg) {
          if (pending_) { feedback_ = Feedback{read_id_, msg.data}; pending_ = false; }
        });
  }
  bool ready() const override { return true; }
  bool supportsSteering() const override { return true; }
  void setVelocity(double velocity, double steering) override {
    steering_ = steering;
    geometry_msgs::msg::Twist msg;
    msg.linear.x = velocity * radius_;
    msg.angular.x = steering;
    tx_->publish(msg);
  }
  void stop() override { setVelocity(0, steering_); }
  void requestFeedback(uint64_t command_id, double) override {
    if (!pending_) { read_id_ = command_id; pending_ = true; }
  }
  std::optional<Feedback> takeFeedback() override {
    auto value = feedback_; feedback_.reset(); return value;
  }
  void tick(double) override {}
 private:
  double radius_;
  double steering_ = 0;
  bool pending_ = false;
  uint64_t read_id_ = 0;
  std::optional<Feedback> feedback_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr tx_;
  rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr rx_;
};

class WheelMotor : public rclcpp::Node {
 public:
  WheelMotor() : Node("wheel_motor") {
    const auto wheel = declare_parameter<std::string>("wheel", "FrontLeft");
    if (wheel.empty() || wheel.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_")
        != std::string::npos) { throw std::invalid_argument("Invalid wheel topic name"); }
    MotorConfig config;
    config.max_velocity = declare_parameter("max_velocity", config.max_velocity);
    config.command_timeout = declare_parameter("command_timeout", config.command_timeout);
    config.feedback_timeout = declare_parameter("feedback_timeout", config.feedback_timeout);
    config.settle_timeout = declare_parameter("settle_timeout", config.settle_timeout);
    config.velocity_tolerance = declare_parameter("velocity_tolerance", config.velocity_tolerance);
    config.update_period = declare_parameter("update_period", config.update_period);
    const auto backend = declare_parameter<std::string>("backend", "fake");
    if (backend == "fake") {
      backend_ = std::make_unique<FakeMotorBackend>();
    } else if (backend == "can") {
      if (!declare_parameter("enable_motor", false)) {
        throw std::invalid_argument("CAN backend requires enable_motor:=true and confirmed hardware configuration");
      }
      const int uuid = declare_parameter("motor_uuid", -1);
      const double gear_ratio = declare_parameter("gear_ratio", 0.0);
      const int direction = declare_parameter("direction", 1);
      const int encoder_id = declare_parameter("encoder_id", 0);
      const double response_timeout = declare_parameter("response_timeout", 0.2);
      backend_ = std::make_unique<CanMotorBackend>(
          *this, uuid, gear_ratio, direction, encoder_id, response_timeout);
    } else if (backend == "sim") {
      const double radius = declare_parameter("wheel_radius", 0.15);
      if (!std::isfinite(radius) || radius <= 0) { throw std::invalid_argument("Invalid wheel radius"); }
      backend_ = std::make_unique<SimMotorBackend>(*this, wheel, radius);
    } else { throw std::invalid_argument("backend must be fake, can or sim"); }
    controller_ = std::make_unique<MotorController>(*backend_, config);
    const auto topic = "/Drive/" + wheel;
    status_ = create_publisher<drive_interfaces::msg::MotorStatus>(topic + "/status", 10);
    commands_ = create_subscription<drive_interfaces::msg::WheelCommand>(topic, rclcpp::QoS(1),
        [this](const drive_interfaces::msg::WheelCommand & msg) {
          controller_->setVelocity(msg.velocity, msg.steering_angle, steadyNow());
        });
    timer_ = create_wall_timer(std::chrono::milliseconds(20), [this]() { tick(); });
    RCLCPP_INFO(get_logger(), "%s motor backend: %s", wheel.c_str(), backend.c_str());
  }

  void stop() { backend_->stop(); }

 private:
  static double steadyNow() {
    return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
  }
  void tick() {
    const double time = steadyNow();
    controller_->tick(time);
    const auto & state = controller_->state();
    drive_interfaces::msg::MotorStatus msg;
    msg.stamp = now();
    msg.command_id = state.command_id;
    msg.requested_velocity = state.requested_velocity;
    msg.measured_velocity = state.measured_velocity;
    msg.feedback_age = state.feedback_time < 0 ? -1 : time - state.feedback_time;
    msg.feedback_valid = controller_->feedbackValid(time);
    msg.ready = controller_->ready();
    msg.command_result = static_cast<uint8_t>(state.result);
    msg.fault = state.fault;
    status_->publish(msg);
  }
  std::unique_ptr<MotorBackend> backend_;
  std::unique_ptr<MotorController> controller_;
  rclcpp::Publisher<drive_interfaces::msg::MotorStatus>::SharedPtr status_;
  rclcpp::Subscription<drive_interfaces::msg::WheelCommand>::SharedPtr commands_;
  rclcpp::TimerBase::SharedPtr timer_;
};
}  // namespace rover::drive

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  try {
    auto node = std::make_shared<rover::drive::WheelMotor>();
    rclcpp::spin(node);
    // SIGINT may already have shut down ROS. Hardware watchdog covers that case.
    if (rclcpp::ok()) { node->stop(); }
  } catch (const std::exception & error) {
    RCLCPP_ERROR(rclcpp::get_logger("wheel_motor"), "%s", error.what());
    rclcpp::shutdown();
    return 1;
  }
  rclcpp::shutdown();
  return 0;
}
