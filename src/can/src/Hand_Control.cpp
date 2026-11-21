#include <cmath>
#include <memory>
#include <string>
#include <functional>
#include <rclcpp/rclcpp.hpp>
#include <chrono>
#include <thread>

#include "./../include/can/CAN.h"

#include "std_msgs/msg/float64.hpp"

using std::placeholders::_1;
using std::placeholders::_2;
using namespace rclcpp;
using namespace can; 

enum class boardid_t {
  frontTireLeft,
  frontTireRight,
  rearTireLeft,
  rearTireRight,
  armBase,
  shoulder,
  elbow,
  forearm,
  wristDiffRight,
  wristDiffLeft,
  telemetry,
  hand,
  debug1,
  debug2
};

std::map<boardid_t, CANDevice_t> boardDeviceMap =
	{
	 // Motor (0x30-0x39)
	 {boardid_t::frontTireLeft, CANDevice_t{0, 1, 0, CAN_UUID_BLDC_FRONT_TIRE_LEFT}},
	 {boardid_t::frontTireRight, CANDevice_t{0, 1, 0, CAN_UUID_BLDC_FRONT_TIRE_RIGHT}},
	 {boardid_t::rearTireLeft, CANDevice_t{0, 1, 0, CAN_UUID_BLDC_REAR_TIRE_LEFT}},
	 {boardid_t::rearTireRight, CANDevice_t{0, 1, 0, CAN_UUID_BLDC_REAR_TIRE_RIGHT}},
	 {boardid_t::armBase, CANDevice_t{0, 1, 0, CAN_UUID_BLDC_BASE}},
	 {boardid_t::shoulder, CANDevice_t{0, 1, 0, CAN_UUID_BLDC_SHOULDER}},
	 {boardid_t::elbow, CANDevice_t{0, 1, 0, CAN_UUID_BLDC_ELBOW}},
	 {boardid_t::forearm, CANDevice_t{0, 1, 0, CAN_UUID_BLDC_FOREARM}},
	 {boardid_t::wristDiffLeft, CANDevice_t{0, 1, 0, CAN_UUID_BLDC_WRIST_LEFT}},
	 {boardid_t::wristDiffRight, CANDevice_t{0, 1, 0, CAN_UUID_BLDC_WRIST_RIGHT}},
	
	 // Telemetry (0x50)
	 {boardid_t::telemetry, CANDevice_t{1, 0, 0, CAN_UUID_TELEMETRY}},
	 // Hand (0x60)
	 {boardid_t::hand, CANDevice_t{1, 1, 0, CAN_UUID_HAND}},
	 // DEBUG (0x70, 0x71)
	 {boardid_t::debug1, CANDevice_t{1, 0, 0, CAN_UUID_DEBUG1}},
	 {boardid_t::debug2, CANDevice_t{1, 0, 0, CAN_UUID_DEBUG2}}
	};

// Inspired by https://github.com/ros-controls/ros2_control
class Hand_Control : public rclcpp::Node {
  public:
    Hand_Control() : Node("Hand_Control_node") {
      this->servo_subscriber_ = this->create_subscription<std_msgs::msg::Float64>(
        "/cn/", 10, std::bind(&Hand_Control::set_motor_power, this, _1));
      
     Publisher<can::msg::CANPacket>::SharedPtr ptr = 
          this->create_publisher<can::msg::CANPacket>("can_tx", 10);
    }

    void update() {
      for (auto const& device : motor_to_power) {
        CANDevice_t device_des = CANDevice_t{};
        device_des.peripheralDomain = 0;
        device_des.motorDomain = 1;
        device_des.powerDomain = 0;
        device_des.deviceUUID = device.first;

        CANPacket_t p = CANMotorPacket_BLDC_SetInputVelocity(
            can::JETSON_DEVICE, device_des, device.second, 0.0f
        );

        can::msg::CANPacket ros_can_packet;
        ros_can_packet.device = device_des;
        ros_can_packet.priority = false;
        ros_can_packet.contents_length = p.contentsLength;
        ros_can_packet.sender_uuid = CAN_UUID_JETSON;
        memmove(&ros_can_packet.contents, &p.contents, 6);

        can_publisher->publish(ros_can_packet);
      }
    }

  private:

    void set_motor_power(std_msgs::msg::Float64::SharedPtr power) const {

    }

    // Motor UUID and their initialized power
    std::map<CANDeviceUUID_t, float> motor_to_power = {{CAN_UUID_BLDC_FRONT_TIRE_LEFT, 0.0f},
                                                        {CAN_UUID_BLDC_FRONT_TIRE_RIGHT, 0.0f},
                                                        {CAN_UUID_BLDC_REAR_TIRE_LEFT, 0.0f},
                                                        {CAN_UUID_BLDC_REAR_TIRE_RIGHT, 0.0f},
                                                        {CAN_UUID_BLDC_BASE, 0.0f},
                                                        {CAN_UUID_BLDC_SHOULDER, 0.0f},
                                                        {CAN_UUID_BLDC_ELBOW, 0.0f},
                                                        {CAN_UUID_BLDC_WRIST_RIGHT, 0.0f}};

    
    
    Subscription<std_msgs::msg::Float64>::SharedPtr servo_subscriber_;
    Publisher<can::msg::CANPacket>::SharedPtr can_publisher;
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);

  std::shared_ptr<rclcpp::Executor> executor =
    std::make_shared<rclcpp::executors::MultiThreadedExecutor>();
  std::string manager_node_name = "Hand_Controller";

  auto hc = std::make_shared<Hand_Control>();

  std::thread cm_thread(
    [hc]()
    {
      while (rclcpp::ok())
      {

        // In seconds by multiplying by 1e6
        // Give up CPU time for certain seconds
        std::this_thread::sleep_for(
          std::chrono::microseconds(static_cast<int>(1 * 1e6)));
        RCLCPP_INFO(hc->get_logger(), "Update Motor State");
        hc->update();
      }
    });

  executor->spin();
  cm_thread.join();


  rclcpp::shutdown();
  return 0;
}
