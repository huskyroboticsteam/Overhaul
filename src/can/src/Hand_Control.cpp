#include <cmath>
#include <memory>
#include <string>
#include <functional>
#include <rclcpp/rclcpp.hpp>
#include <chrono>
#include <CAN26.h>
#include <thread>



using std::placeholders::_1;
using namespace rclcpp;

// Inspired by https://github.com/ros-controls/ros2_control
class Hand_Control : public rclcpp::Node {
  public:
    Hand_Control() : Node("Hand_Control_node") {
      // this->servo_subscriber_ = this->create_subscription<geometry_msgs::msg::Twist>(
      //   "Hand/Servo", 10, std::bind(&Hand_Control::set_servo_power, this, _1));
      
    //  Publisher<Twist>::SharedPtr ptr = 
    //       this->create_publisher<geometry_msgs::msg::Twist>(curr_el, 10);
    }
  private:

    // void set_servo_power(const double power) const {

    // }

    // Subscription<double>::SharedPtr servo_subscriber_;
    // Publisher<double>::SharedPtr can_publisher;

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
      }
    });

  executor->spin();
  cm_thread.join();


  rclcpp::shutdown();
  return 0;
}
