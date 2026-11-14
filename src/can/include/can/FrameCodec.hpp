#pragma once

extern "C" {
#include <CANPacket.h>
}
#include <linux/can.h>
#include <cstddef>
#include <cstring>

namespace can {
inline bool decodeFrame(const can_frame & frame, std::size_t size, CANPacket_t & packet) {
  if (size != sizeof(frame) || frame.can_dlc < 2 || frame.can_dlc > 8 ||
      (frame.can_id & (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_ERR_FLAG))) {
    return false;
  }
  // CAN26: [inverted priority:1][UUID:7][power:1][motor:1][peripheral:1].
  packet = {};
  packet.device.deviceUUID = (frame.can_id >> 3) & 0x7f;
  packet.device.peripheralDomain = (frame.can_id & 1) != 0;
  packet.device.motorDomain = (frame.can_id & 2) != 0;
  packet.device.powerDomain = (frame.can_id & 4) != 0;
  packet.priority = (frame.can_id & 0x400) ? CAN_PRIORITY_LOW : CAN_PRIORITY_HIGH;
  packet.command = frame.data[0];
  packet.senderUUID = frame.data[1];
  packet.contentsLength = frame.can_dlc - 2;
  std::memcpy(packet.contents, frame.data + 2, packet.contentsLength);
  return true;
}
}  // namespace can
