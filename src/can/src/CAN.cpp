#include "../include/can/CAN.h"

#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

namespace can {

// time to sleep after getting a CAN read error
constexpr std::chrono::milliseconds READ_ERR_SLEEP(100);

// CAN26 11-bit ID layout: [priority:1][deviceUUID:7][peripheral:1][power:1][motor:1]
// Match on UUID field to filter for packets addressed to this device
constexpr uint32_t CAN_MASK = 0x3F8; // UUID field

CAN::CAN() : Node("CAN_node") {
  auto can_name_desc = rcl_interfaces::msg::ParameterDescriptor{};
  can_name_desc.description = "Specifies the name of the CAN interface, default can0";
  this->declare_parameter("CAN_name", "can0", can_name_desc);
  this->_can_name = this->get_parameter("CAN_name").as_string();

  initCAN();

  receiveThreadFn();
}

void CAN::initCAN() {
  RCLCPP_INFO(this->get_logger(), "Initializing CAN");
  this->_fd = createCANSocket({});
  if (this->_fd < 0) {
    std::__throw_runtime_error("Unable to open CAN connection!");
  }

  // start thread for recieving CAN packets
	// std::thread receiveThread(&CAN::receiveThreadFn);
	// receiveThread.detach();

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int CAN::createCANSocket(std::optional<CANDevice_t> device) {
  int fd;
  if ((fd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
    RCLCPP_ERROR(this->get_logger(), "Failed to initialize CAN bus: %s", std::strerror(errno));
    return -1;
  }

  struct ifreq ifr;
	std::strcpy(ifr.ifr_name, _can_name.c_str());
	if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
    RCLCPP_ERROR(this->get_logger(), "Failed to get hardware CAN interface index: %s", std::strerror(errno));
		std::strcpy(ifr.ifr_name, (std::string("v") + _can_name).c_str());
		if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
      RCLCPP_ERROR(this->get_logger(), "Failed to get virtual CAN interface index: %s", std::strerror(errno));
      return -1;
		}
    RCLCPP_INFO(this->get_logger(), "Found virtual CAN interface index.");
	}

	struct sockaddr_can addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    RCLCPP_ERROR(this->get_logger(), "Error binding CAN socket: %s", std::strerror(errno));
    return -1;
	}

	// enable reception at the given device, if provided
	if (device) {
		// Build CAN ID from CANDevice_t using CAN26's packet header format
		CANPacket_t dummy = {};
		dummy.device = *device;
		dummy.priority = CAN_PRIORITY_LOW;
		uint16_t canID = CANGetPacketHeader(&dummy);
		can_filter filters[1];
		filters[0].can_id = canID;
		filters[0].can_mask = CAN_MASK;

		setsockopt(fd, SOL_CAN_RAW, CAN_RAW_FILTER, &filters, sizeof(filters));
	} else {
		// disable reception on this socket.
		setsockopt(fd, SOL_CAN_RAW, CAN_RAW_FILTER, nullptr, 0);
	}

  return fd;
}

void CAN::receiveThreadFn() {
	// create dedicated CAN socket for reading
	int recvFD = createCANSocket(CANDevice_t{0, 0, 0, CAN_UUID_JETSON});
	if (recvFD < 0) {
    RCLCPP_ERROR(this->get_logger(), "Unable to open CAN connection!");
		return;
	}

  CANPacket_t packet;
	while (true) {
		// no synchronization necessary, since this thread owns the FD
		if (receivePacket(recvFD, packet)) {
      RCLCPP_INFO(this->get_logger(), "Packet received!");
			// Add packet to buffer
			// std::unique_lock lock(bufferMutex);
			// buffer.push(packet);
			// lock.unlock();
		} else {
			// we had a bus error, so sleep for a bit
			std::this_thread::sleep_for(READ_ERR_SLEEP);
		}
	}
}

bool CAN::receivePacket(int fd, CANPacket_t& packet) {
	int ret;
	can_frame frame;
	ret = read(fd, &frame, sizeof(can_frame));
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
  rclcpp::spin(std::make_shared<can::CAN>());
  rclcpp::shutdown();
  return 0;
}
