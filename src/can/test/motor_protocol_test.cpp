#include "can/MotorProtocol.hpp"
#include "can/FrameCodec.hpp"
#include <iostream>

#define CHECK(expr) do { if (!(expr)) { \
  std::cerr << "Failed at line " << __LINE__ << ": " << #expr << '\n'; return 1; \
} } while (false)

int main() {
  CANDevice_t sender{};
  sender.deviceUUID = 1;
  CANDevice_t motor{};
  motor.deviceUUID = 0x30;
  motor.motorDomain = true;
  for (double sign : {-1.0, 0.0, 1.0}) {
    const auto packet = can::motor::velocityPacket(sender, motor, sign * can::motor::TWO_PI, 1, 1);
    CHECK(packet.command == 0x89 && packet.contentsLength == 6 && packet.senderUUID == 1);
    CHECK(CANGetPacketHeader(&packet) == 0x582);
    CHECK(CANGetDlc(&packet) == 8);
    const auto * data = CANGetDataConst(&packet);
    CHECK(data[0] == 0x89 && data[1] == 1);
    CHECK(data[2] == 0 && data[3] == 0);
    CHECK(data[4] == (sign == 0 ? 0 : 0x80));
    CHECK(data[5] == (sign < 0 ? 0xbf : sign > 0 ? 0x3f : 0));
    CHECK(data[6] == 0 && data[7] == 0);
  }
  auto geared = can::motor::velocityPacket(sender, motor, can::motor::TWO_PI, 10, -1);
  CHECK(CANMotorPacket_BLDC_SetInputVelocity_Decode(&geared).velocity == -10);
  const auto feedback = CANMotorPacket_BLDC_EncoderEstimates(motor, sender, 3, -10);
  CHECK(std::abs(can::motor::wheelVelocity(feedback, 10, -1) - can::motor::TWO_PI) < 1e-6);
  auto malformed = feedback;
  malformed.contentsLength = 5;
  bool rejected = false;
  try { can::motor::wheelVelocity(malformed, 10, 1); }
  catch (const std::invalid_argument &) { rejected = true; }
  CHECK(rejected);
  can_frame frame{};
  frame.can_id = 0x40a;  // Jetson, motor domain, low priority.
  frame.can_dlc = 8;
  frame.data[0] = 0x0e;
  frame.data[1] = 0x30;
  CANPacket_t decoded{};
  CHECK(can::decodeFrame(frame, sizeof(frame), decoded));
  CHECK(decoded.device.deviceUUID == 1 && decoded.device.motorDomain);
  CHECK(!decoded.device.powerDomain && !decoded.device.peripheralDomain);
  CHECK(decoded.priority == CAN_PRIORITY_LOW && decoded.senderUUID == 0x30);
  CHECK(decoded.command == 0x0e && decoded.contentsLength == 6);
  frame.can_id = 0x00f;
  CHECK(can::decodeFrame(frame, sizeof(frame), decoded));
  CHECK(decoded.priority == CAN_PRIORITY_HIGH && decoded.device.powerDomain &&
      decoded.device.peripheralDomain && decoded.device.motorDomain);
  CHECK(!can::decodeFrame(frame, sizeof(frame) - 1, decoded));
  for (auto flag : {CAN_EFF_FLAG, CAN_RTR_FLAG, CAN_ERR_FLAG}) {
    frame.can_id = 0x40a | flag;
    CHECK(!can::decodeFrame(frame, sizeof(frame), decoded));
  }
  frame.can_id = 0x40a;
  for (auto length : {0, 1, 9}) {
    frame.can_dlc = length;
    CHECK(!can::decodeFrame(frame, sizeof(frame), decoded));
  }
  std::cout << "Motor protocol tests passed\n";
  return 0;
}
