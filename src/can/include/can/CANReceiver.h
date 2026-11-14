#pragma once

#include "./CAN.h"

namespace can {

class CANReceiver : public rclcpp::Node {
  public:
    CANReceiver();
    ~CANReceiver() override;

  private:
    /**
     * @brief Recieves CAN packets from the file descriptor
     * 
     * @param fd the file descriptor to read from
     * @param packet packet that will get written to once read
     * 
     * @return true if read a packet was received successfully, false otherwise
     */
    bool receivePacket(int fd, CANPacket_t& packet);

    rclcpp::Publisher<can::msg::CANPacket>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    int fd_ = -1;
};

} // namespace can
