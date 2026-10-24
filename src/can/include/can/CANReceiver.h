#pragma once

#include "./CAN.h"

namespace can {

class CANReceiver : public rclcpp::Node {
  public:
    CANReceiver();

  private:
    void publishPacket();

    /**
     * @brief Recieves CAN packets from the file descriptor
     * 
     * @param fd the file descriptor to read from
     * @param packet packet that will get written to once read
     * 
     * @return true if read a packet was received successfully, false otherwise
     */
    bool receivePacket(CANPacket_t& packet);

    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<can::msg::CANPacket>::SharedPtr publisher_;
    std::string can_name_;
    int fd_;
};

} // namespace can