#include "can/CANSender.h"
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>

namespace can {
CANSender::CANSender() : Node("CAN_sender_node") {
  const auto name = declare_parameter<std::string>("CAN_name", "can0");
  fd_ = createCANSocket(name, get_logger(), std::nullopt);
  if (fd_ < 0) { throw std::runtime_error("Unable to open CAN send connection"); }
  const int flags = fcntl(fd_, F_GETFL, 0);
  if (flags < 0 || fcntl(fd_, F_SETFL, flags | O_NONBLOCK) < 0) {
    close(fd_);
    throw std::runtime_error("Unable to configure nonblocking CAN send socket");
  }
  subscription_ = create_subscription<can::msg::CANPacket>("can_tx", rclcpp::QoS(10),
      std::bind(&CANSender::sendCANPacket, this, std::placeholders::_1));
}

CANSender::~CANSender() { if (fd_ >= 0) { close(fd_); } }

void CANSender::sendCANPacket(const can::msg::CANPacket::SharedPtr msg) const {
  if (msg->contents_length > msg->contents.size() || msg->device.device_uuid > 127 ||
      msg->sender_uuid > 127) {
    RCLCPP_ERROR(get_logger(), "Invalid CAN packet length or UUID");
    return;
  }
  CANPacket_t packet{};
  packet.device.deviceUUID = msg->device.device_uuid;
  packet.device.motorDomain = msg->device.motor;
  packet.device.peripheralDomain = msg->device.peripheral;
  packet.device.powerDomain = msg->device.power;
  packet.priority = CANPriority_t(msg->priority);
  packet.command = msg->command;
  packet.senderUUID = msg->sender_uuid;
  packet.contentsLength = msg->contents_length;
  std::memcpy(packet.contents, msg->contents.data(), msg->contents_length);

  can_frame frame{};
  frame.can_id = CANGetPacketHeader(&packet);
  frame.can_dlc = CANGetDlc(&packet);
  std::memcpy(frame.data, CANGetData(&packet), frame.can_dlc);
  if (!sendCANFrame(frame)) {
    RCLCPP_ERROR(get_logger(), "Failed to send CAN packet to uuid=%x: %s",
        msg->device.device_uuid, std::strerror(errno));
  }
}

bool CANSender::sendCANFrame(const can_frame & frame) const {
  return write(fd_, &frame, sizeof(frame)) == sizeof(frame);
}
}  // namespace can

int main(int argc, char ** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<can::CANSender>());
  rclcpp::shutdown();
  return 0;
}
