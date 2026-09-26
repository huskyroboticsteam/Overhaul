#pragma once

extern "C" {
#include <CAN26.h>
}

#include <rclcpp/rclcpp.hpp>

/**
 * @namespace can
 * @brief Utilities for interacting with CAN devices.
 */
namespace can {

class CAN : public rclcpp::Node {
  public:
    CAN();

    /**
    * @brief Initialize the CAN interface.
    * 
    * This should only be called once.
    * 
    * @note If CAN initialization fails, the program will exit.
    */
    void initCAN();

  private:
    /**
     * @brief Creates the CAN Socket
     * 
     * @param device optional, will enable reception if provided
     * 
     * @return a file descriptor, or -1 on failure
     */
    int createCANSocket(std::optional<CANDevice_t> device);

    /**
     * @brief Thread function to read CAN packets from the network
     */
    void receiveThreadFn();

    /**
     * @brief Recieves CAN packets from the file descriptor
     * 
     * @param fd the file descriptor to read from
     * @param packet packet that will get written to once read
     * 
     * @return true if read a packet was received successfully, false otherwise
     */
    bool receivePacket(int fd, CANPacket_t& packet);

    std::string _can_name;
    int _fd;
};

} // namespace can