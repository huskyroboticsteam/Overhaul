#pragma once

#include "rover/MotorController.hpp"
#include <can/MotorProtocol.hpp>
#include <can/msg/can_packet.hpp>
#include <rclcpp/rclcpp.hpp>
#include <algorithm>

namespace rover::drive {

class CanMotorBackend : public MotorBackend {
 public:
  CanMotorBackend(rclcpp::Node & node, int uuid, double gear_ratio, int direction,
      int encoder_id, double response_timeout)
      : gear_ratio_(gear_ratio), direction_(direction), encoder_id_(encoder_id),
        response_timeout_(response_timeout) {
    if (uuid < 2 || uuid > 127 || encoder_id < 0 || encoder_id > 255 ||
        !std::isfinite(response_timeout) || response_timeout <= 0) {
      throw std::invalid_argument("Set a non-broadcast motor_uuid (2..127), encoder_id and response_timeout");
    }
    can::motor::motorRevsPerSecond(0, gear_ratio, direction);
    motor_.deviceUUID = uuid;
    motor_.motorDomain = true;
    sender_.deviceUUID = 1;  // CAN_UUID_JETSON; matches CANReceiver's receive filter.
    tx_ = node.create_publisher<can::msg::CANPacket>("/can_tx", rclcpp::QoS(10));
    rx_ = node.create_subscription<can::msg::CANPacket>("/can_rx", rclcpp::QoS(20),
        [this](const can::msg::CANPacket & msg) { receive(msg); });
  }

  bool ready() const override { return stage_ == Stage::Ready && fault_.empty(); }
  std::string fault() const override { return fault_; }
  void setVelocity(double velocity, double) override {
    if (ready()) { publish(can::motor::velocityPacket(sender_, motor_, velocity, gear_ratio_, direction_)); }
  }
  void stop() override {
    // Never automatically re-enable a motor after a failure. Zero remains best effort;
    // a configured ODrive watchdog is required for process/bus loss.
    publish(can::motor::velocityPacket(sender_, motor_, 0, gear_ratio_, direction_, false));
  }
  void requestFeedback(uint64_t command_id, double now) override {
    if (!ready() || pending_read_) { return; }
    pending_read_ = true;
    read_id_ = command_id;
    read_deadline_ = now + response_timeout_;
    publish(CANMotorPacket_BLDC_GetEncoderEstimates(sender_, motor_, encoder_id_));
  }
  std::optional<Feedback> takeFeedback() override {
    auto sample = feedback_;
    feedback_.reset();
    return sample;
  }
  void tick(double now) override {
    if (!fault_.empty()) { return; }
    if (stage_ == Stage::Connect) {
      if (deadline_ < 0) { deadline_ = now + 5.0; }
      if (tx_->get_subscription_count() > 0) {
        stop();
        auto packet = CANMotorPacket_BLDC_SetInputMode(
            sender_, motor_, BLDC_VELOCITY_CONTROL, BLDC_PASSTHROUGH_INPUT);
        packet.command = CAN_ACK(packet.command);
        publish(packet);
        stage_ = Stage::ModeAck;
        deadline_ = now + response_timeout_;
      }
    } else if (stage_ == Stage::Enable) {
      auto packet = CANMotorPacket_BLDC_SetAxisState(sender_, motor_, BLDC_AXIS_CLOSED_LOOP_CONTROL);
      packet.command = CAN_ACK(packet.command);
      publish(packet);
      stage_ = Stage::AxisAck;
      deadline_ = now + response_timeout_;
    }
    if (stage_ != Stage::Ready && now >= deadline_) {
      fault_ = "CAN initialization timed out (transport, control mode or axis ACK missing)";
      stop();
    }
    if (pending_read_ && now >= read_deadline_) {
      // No wire transaction ID: do not reuse a timed-out read slot for a new
      // command, since a late response could be mistaken for its confirmation.
      pending_read_ = false;
      fault_ = "CAN encoder read timed out; restart after checking the bus";
      stop();
    }
  }

 private:
  enum class Stage { Connect, ModeAck, Enable, AxisAck, Ready };
  void publish(const CANPacket_t & packet) {
    can::msg::CANPacket msg;
    msg.device.device_uuid = packet.device.deviceUUID;
    msg.device.motor = packet.device.motorDomain;
    msg.device.peripheral = packet.device.peripheralDomain;
    msg.device.power = packet.device.powerDomain;
    msg.priority = packet.priority;
    msg.command = packet.command;
    msg.sender_uuid = packet.senderUUID;
    msg.contents_length = packet.contentsLength;
    std::copy_n(packet.contents, packet.contentsLength, msg.contents.begin());
    tx_->publish(msg);
  }
  void receive(const can::msg::CANPacket & msg) {
    if (!fault_.empty() || msg.sender_uuid != motor_.deviceUUID ||
        msg.device.device_uuid != sender_.deviceUUID || msg.contents_length > 6) { return; }
    if (msg.command == CAN_COMMAND_ID__HEARTBEAT && msg.contents_length == 5 &&
        CANLoadUInt32(msg.contents.data()) != 0) {
      fault_ = "Motor heartbeat reports a hardware error";
      stop();
      return;
    }
    if (msg.command == CAN_COMMAND_ID__ACKNOWLEDGE && msg.contents_length == 2) {
      const auto command = msg.contents[1];
      const bool expected = (stage_ == Stage::ModeAck && command == CAN_COMMAND_ID__BLDC_INPUT_MODE) ||
          (stage_ == Stage::AxisAck && command == CAN_COMMAND_ID__BLDC_AXIS_STATE);
      if (msg.contents[0] != 0 &&
          (expected || (ready() && command == CAN_COMMAND_ID__BLDC_INPUT_VELOCITY))) {
        fault_ = "Motor rejected a CAN command";
        stop();
      } else if (expected && msg.contents[0] == 0) {
        stage_ = stage_ == Stage::ModeAck ? Stage::Enable : Stage::Ready;
      }
      // Generic velocity ACKs cannot identify a particular setpoint. They never
      // mark a command VERIFIED; encoder readback is required, ACK or no ACK.
      return;
    }
    if (ready() && pending_read_ && msg.command == CAN_COMMAND_ID__BLDC_ENCODER_ESTIMATE &&
        msg.contents_length == 6) {
      CANPacket_t packet{};
      packet.command = msg.command;
      packet.contentsLength = msg.contents_length;
      std::copy_n(msg.contents.begin(), 6, packet.contents);
      const double velocity = can::motor::wheelVelocity(packet, gear_ratio_, direction_);
      if (!std::isfinite(velocity)) {
        fault_ = "Non-finite encoder velocity";
        stop();
        return;
      }
      feedback_ = Feedback{read_id_, velocity};
      pending_read_ = false;
    }
  }

  CANDevice_t sender_{};
  CANDevice_t motor_{};
  double gear_ratio_;
  int direction_;
  uint8_t encoder_id_;
  double response_timeout_;
  Stage stage_ = Stage::Connect;
  double deadline_ = -1;
  bool pending_read_ = false;
  double read_deadline_ = 0;
  uint64_t read_id_ = 0;
  std::optional<Feedback> feedback_;
  std::string fault_;
  rclcpp::Publisher<can::msg::CANPacket>::SharedPtr tx_;
  rclcpp::Subscription<can::msg::CANPacket>::SharedPtr rx_;
};
}  // namespace rover::drive
