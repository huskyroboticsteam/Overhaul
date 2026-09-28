#include "../include/can/CAN.h"

#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

namespace can {

// CAN26 11-bit ID layout: [priority:1][deviceUUID:7][peripheral:1][power:1][motor:1]
// Match on UUID field to filter for packets addressed to this device
constexpr uint32_t CAN_MASK = 0x3F8; // UUID field

int createCANSocket(std::string can_name, std::optional<CANDevice_t> device) {
  int fd;
  if ((fd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
    RCLCPP_ERROR(this->get_logger(), "Failed to initialize CAN bus: %s", std::strerror(errno));
    return -1;
  }

  struct ifreq ifr;
	std::strcpy(ifr.ifr_name, can_name.c_str());
	if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
    RCLCPP_ERROR(this->get_logger(), "Failed to get hardware CAN interface index: %s", std::strerror(errno));
		std::strcpy(ifr.ifr_name, (std::string("v") + can_name).c_str());
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

} // namespace can