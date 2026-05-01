#include <linux/can.h>
#include <net/if.h>
#include <rclcpp/rclcpp.hpp>
#include <sys/ioctl.h>
#include <sys/socket.h>

class CAN : public rclcpp::Node {
  public:
    CAN() : Node("can_node") {
      if ((can_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        RCLCPP_ERROR(this->get_logger(), "Failed to initialize CAN bus: %s", std::strerror(errno));
      }

      struct ifreq ifr;
      std::strcpy(ifr.ifr_name, "can0");
      if (ioctl(can_fd, SIOCGIFINDEX, &ifr) < 0) {
        
      }
    }
  private:
    int can_fd;
};