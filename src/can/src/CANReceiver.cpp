#include "can/CANReceiver.h"
#include "can/FrameCodec.hpp"
#include <linux/can/raw.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <stdexcept>

namespace can {
CANReceiver::CANReceiver() : Node("CAN_receiver_node") {
  const auto name = declare_parameter<std::string>("CAN_name", "can0");
  publisher_ = create_publisher<can::msg::CANPacket>("can_rx", rclcpp::QoS(100));
  fd_ = createCANSocket(name, get_logger(), CANDevice_t{0, 0, 0, CAN_UUID_JETSON});
  if (fd_ < 0) { throw std::runtime_error("Unable to open CAN receive connection"); }
  const int flags = fcntl(fd_, F_GETFL, 0);
  if (flags < 0 || fcntl(fd_, F_SETFL, flags | O_NONBLOCK) < 0) {
    close(fd_);
    throw std::runtime_error("Unable to configure nonblocking CAN receive socket");
  }
  timer_ = create_wall_timer(std::chrono::milliseconds(5), [this]() {
    // Bound work per tick so shutdown and other executor callbacks can run.
    for (int i = 0; i < 64; ++i) {
      CANPacket_t packet{};
      if (!receivePacket(fd_, packet)) { break; }
      can::msg::CANPacket msg;
      msg.device.device_uuid = packet.device.deviceUUID;
      msg.device.motor = packet.device.motorDomain;
      msg.device.peripheral = packet.device.peripheralDomain;
      msg.device.power = packet.device.powerDomain;
      msg.priority = packet.priority;
      msg.command = packet.command;
      msg.sender_uuid = packet.senderUUID;
      msg.contents_length = packet.contentsLength;
      std::memcpy(msg.contents.data(), packet.contents, packet.contentsLength);
      publisher_->publish(msg);
    }
  });
}

CANReceiver::~CANReceiver() { if (fd_ >= 0) { close(fd_); } }

bool CANReceiver::receivePacket(int fd, CANPacket_t & packet) {
  can_frame frame{};
  const auto size = read(fd, &frame, sizeof(frame));
  if (size < 0) {
    if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
      RCLCPP_ERROR_THROTTLE(get_logger(), *get_clock(), 1000,
          "CAN read failed: %s", std::strerror(errno));
    }
    return false;
  }
  if (!decodeFrame(frame, static_cast<std::size_t>(size), packet)) {
    RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 1000, "Ignoring malformed/non-data CAN frame");
    return false;
  }
  return true;
}
}  // namespace can

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<can::CANReceiver>());
  rclcpp::shutdown();
  return 0;
}
