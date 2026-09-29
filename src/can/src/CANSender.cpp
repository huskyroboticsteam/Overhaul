#include "../include/can/CANSender.h"

#include <memory>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <termios.h>

namespace can {

CANSender::CANSender() : Node("CAN_sender_node") {
	auto can_name_desc = rcl_interfaces::msg::ParameterDescriptor{};
	can_name_desc.description = "Specifies the name of the CAN interface, default can0";
	this->declare_parameter("CAN_name", "can0", can_name_desc);
	std::string can_name = this->get_parameter("CAN_name").as_string();

    this->subscription_ = this->create_subscription<can::msg::CANPacket>(
        "can_tx", rclcpp::QoS(rclcpp::KeepAll()), std::bind(&CANSender::sendCANPacket, this, std::placeholders::_1));

    this->fd_ = createCANSocket(can_name, this->get_logger(), CANDevice_t{});
    
    if (this->fd_ < 0) {
        std::__throw_runtime_error("Unable to open CAN connection!");
    }
}

void CANSender::sendCANPacket(const can::msg::CANPacket::SharedPtr msg) const {
    CANPacket_t packet = CANPacket_t{};
    CANDevice_t device = CANDevice_t{};
    device.deviceUUID = msg->device.device_uuid;
    device.motorDomain = msg->device.motor;
    device.peripheralDomain = msg->device.peripheral;
    device.powerDomain = msg->device.power;

    packet.device = device;
    packet.priority = CANPriority_t(msg->priority);
    packet.contentsLength = msg->contents_length;
    packet.command = msg->command;
    packet.senderUUID = msg->sender_uuid;
    for (int i = 0; i < msg->contents_length; i++) {
        packet.contents[i] = msg->contents[i];
    }

    canfd_frame frame;
    std::memset(&frame, 0, sizeof(frame));
    frame.can_id = CANGetPacketHeader(&packet);
    frame.len = CANGetDlc(&packet);
    std::memcpy(frame.data, CANGetData(&packet), frame.len);

    bool success = sendCANFrame(frame);

    if (!success) {
        RCLCPP_ERROR(this->get_logger(), "Failed to send CAN packet to uuid=%x: %s", msg->device.device_uuid, std::strerror(errno));
    }
}

bool CANSender::sendCANFrame(const canfd_frame& frame) const {
    bool success = write(this->fd_, &frame, sizeof(struct can_frame)) == sizeof(struct can_frame);
    tcdrain(this->fd_);
    return success;
}

} // namespace can

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<can::CANSender>());
  rclcpp::shutdown();
  return 0;
}
