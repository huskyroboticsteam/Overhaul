#include <chrono>
#include <functional>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "publisher.hpp"

using namespace std::chrono_literals;

ZedCamPublisher::ZedCamPublisher()
    : Node("zed_cam_node")
{
    init_params_ = get_camera_init_params();
    
    zed_cam_.open(init_params_);
    publisher_right_cam_ =
        create_publisher<sensor_msgs::msg::Image>("/video/zed/right", 10);

    timer_right_cam_ = create_wall_timer(
        1000ms / init_params_.camera_fps,
        std::bind(&ZedCamPublisher::right_cam_callback, this));
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ZedCamPublisher>());
    rclcpp::shutdown();
    return 0;
}
