#include "../include/can/CANReceiver.h"

#include <linux/can/raw.h>

namespace can {

// time to sleep after getting a CAN read error
constexpr std::chrono::milliseconds READ_ERR_SLEEP(100);

CANReceiver::CANReceiver() : Node("CAN_receiver_node") {
	auto can_name_desc = rcl_interfaces::msg::ParameterDescriptor{};
	can_name_desc.description = "Specifies the name of the CAN interface, default can0";
	this->declare_parameter("CAN_name", "can0", can_name_desc);
	std::string can_name = this->get_parameter("CAN_name").as_string();

	this->publisher_ = this->create_publisher<can::msg::CANPacket>("can/rx", rclcpp::QoS(rclcpp::KeepAll()));

	// create dedicated CAN socket for reading
	int fd = createCANSocket(can_name, this->get_logger(), CANDevice_t{0, 0, 0, CAN_UUID_JETSON});
	if (fd < 0) {
		RCLCPP_ERROR(this->get_logger(), "Unable to open CAN connection!");
		return;
	}

	timer_ = this->create_wall_timer(std::chrono::nanoseconds(0), std::bind(&CANReceiver::publishPacket, this));
}

void CANReceiver::publishPacket() {
	CANPacket_t packet;
	// no synchronization necessary, since this thread owns the FD
	if (receivePacket(packet)) {
		// Copy packet into message
		can::msg::CANPacket message = can::msg::CANPacket();
		can::msg::CANDevice device = can::msg::CANDevice();
		device.device_uuid = packet.device.deviceUUID;
		device.motor = packet.device.motorDomain;
		device.peripheral = packet.device.peripheralDomain;
		device.power = packet.device.powerDomain;

		message.device = device;
		message.priority = packet.priority;
		message.contents_length = packet.contentsLength;
		message.command = packet.command;
		message.sender_uuid = packet.senderUUID;
		for (int i = 0; i < packet.contentsLength; i++) {
			message.contents[i] = packet.contents[i];
		}

		// Publish message
		this->publisher_->publish(message);
	} else {
		// we had a bus error, so sleep for a bit
		std::this_thread::sleep_for(READ_ERR_SLEEP);
	}
}

bool CANReceiver::receivePacket(CANPacket_t& packet) {
	int ret;
	can_frame frame;
	ret = read(this->fd_, &frame, sizeof(can_frame));
	if (ret >= 0) {
		// Parse 11-bit CAN ID into CANDevice_t + priority
		// [priority:1][deviceUUID:7][peripheral:1][power:1][motor:1]
		uint16_t canID = frame.can_id & 0x7FF; // extract the 11-bit CAN ID from frame
		uint16_t deviceBits = canID & 0x3FF;   // extract lower 10 bits (device info)
		std::memcpy(&packet.device, &deviceBits, sizeof(uint16_t));
		packet.priority = (canID & 0x400) ? CAN_PRIORITY_LOW : CAN_PRIORITY_HIGH;

		// Parse 8-byte CAN data
		// [command:1][senderUUID:1][contents:0-6]
		if (frame.can_dlc >= 2) {
			packet.command = frame.data[0];
			packet.senderUUID = frame.data[1];
			packet.contentsLength = frame.can_dlc - 2;
			std::memcpy(packet.contents, frame.data + 2, packet.contentsLength);
		} else {
			packet.command = 0;
			packet.senderUUID = 0;
			packet.contentsLength = 0;
		}
		return true;
	} else {
		RCLCPP_ERROR(this->get_logger(), "Failed to receive CAN packet: %s", std::strerror(errno));
		return false;
	}
}

} // namespace can

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);

  rclcpp::executors::SingleThreadedExecutor executor;
  rclcpp::Node::SharedPtr node = std::make_shared<can::CANReceiver>();
  executor.add_node(node);

	std::thread thread = std::thread([&]() { executor.spin(); });

	while (rclcpp::ok()) {}

  rclcpp::shutdown();
  thread.join();

  return 0;
}
