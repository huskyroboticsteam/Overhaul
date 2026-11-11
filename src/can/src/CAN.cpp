#include "../include/can/CAN.h"

#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

namespace can {

// CAN26 11-bit ID layout: [priority:1][deviceUUID:7][peripheral:1][power:1][motor:1]
// Match on UUID field to filter for packets addressed to this device
constexpr uint32_t CAN_MASK = 0x3F8;  // UUID field

int createCANSocket(std::string can_name, std::optional<rclcpp::Logger> logger, std::optional<CANDevice_t> device) {
  rclcpp::Logger log = logger.value_or(rclcpp::get_logger("CAN"));
  int fd;
  if ((fd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
    RCLCPP_ERROR(log, "Failed to initialize CAN bus: %s", std::strerror(errno));
    return -1;
  }

  struct ifreq ifr;
  std::strcpy(ifr.ifr_name, can_name.c_str());
  if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
    RCLCPP_ERROR(log, "Failed to get hardware CAN interface index: %s", std::strerror(errno));
    std::strcpy(ifr.ifr_name, (std::string("v") + can_name).c_str());
    if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
      RCLCPP_ERROR(log, "Failed to get virtual CAN interface index: %s", std::strerror(errno));
      return -1;
    }
    RCLCPP_INFO(log, "Found virtual CAN interface index.");
  }

  struct sockaddr_can addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;

  if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    RCLCPP_ERROR(log, "Error binding CAN socket: %s", std::strerror(errno));
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

can::msg::CANDevice can2ros_device(CANDevice_t device) {
  return can::build<can::msg::CANDevice>()
      .device_uuid(device.deviceUUID)
      .motor(device.motorDomain)
      .peripheral(device.peripheralDomain)
      .power(device.powerDomain);
}

CANDevice_t ros2can_device(can::msg::CANDevice device) {
  return CANDevice_t{ device.peripheral, device.motor, device.power, device.device_uuid };
}

can::msg::CANPacket can2ros_packet(CANPacket_t packet) {
  std::array<uint8_t, 6UL> array;
  for (int i = 0; i < 6; i++) {
    array[i] = packet.contents[i];
  }

  return can::build<can::msg::CANPacket>()
      .device(can2ros_device(packet.device))
      .priority(packet.priority)
      .contents_length(packet.contentsLength)
      .command(packet.command)
      .sender_uuid(packet.senderUUID)
      .contents(array);
}

CANPacket_t ros2can_packet(can::msg::CANPacket packet) {
  return CANPacket_t{ ros2can_device(packet.device),
                      (CANPriority_t)packet.priority,
                      packet.contents_length,
                      packet.command,
                      packet.sender_uuid,
                      { packet.contents[0], packet.contents[1], packet.contents[2], packet.contents[3],
                        packet.contents[4], packet.contents[5] } };
}

can::msg::CANPacket createMotorVelocityPacket(can::msg::CANDevice device, double vel) {
  CANDevice_t jetson = CANDevice_t{ 0, 0, 0, 0x1 };

  CANPacket_t packet = CANMotorPacket_BLDC_SetInputVelocity(jetson, ros2can_device(device), vel, 0.0f);

  return can2ros_packet(packet);
}

}  // namespace can