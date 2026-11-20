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
    zed_cam_ = sl::Camera();
    init_params_ = sl::InitParameters();
    init_params_.camera_resolution = sl::RESOLUTION::HD1200;
    init_params_.camera_fps = 60;
    init_params_.depth_mode = sl::DEPTH_MODE::NEURAL_PLUS;
    init_params_.coordinate_units_ = sl::UNIT::METER;
    init_params_.coordinate_system_ = sl::COORDINATE_SYSTEM::IMAGE;

    zed_cam_.open(init_params_);
    publisher_right_cam_ =
        create_publisher<sensor_msgs::msg::Image>("/video/zed/right", 10);

    timer_right_cam_ = create_wall_timer(
        100ms,
        std::bind(&ZedCamPublisher::right_cam_callback, this));
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ZedCamPublisher>());
    rclcpp::shutdown();
    return 0;
}
