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
    sl::ERROR_CODE err = zed_cam_.open(init_params_);
    if (err != sl::ERROR_CODE::SUCCESS) {
        RCLCPP_ERROR(get_logger(), "Error: Could not open the ZED camera. Received error code: %s", interpret_error_code(err).c_str());
        rclcpp::shutdown();
    }
    std::map<sl::VIDEO_SETTINGS, int> video_settings = get_video_settings();
    for (const std::pair<sl::VIDEO_SETTINGS, int>& pair : video_settings) {
        zed_cam_.setCameraSettings(pair.first, pair.second);
    }
    runtime_params_ = get_runtime_params();
    zed_cam_.enablePositionalTracking(get_positional_tracking_params());
    zed_cam_.enableObjectDetection(get_object_detection_params());
    publisher_right_cam_ = create_publisher<sensor_msgs::msg::Image>("/video/zed/right", 10);
    publisher_left_cam_ = create_publisher<sensor_msgs::msg::Image>("/video/zed/left", 10);

    timer_cam_image_publisher_ = create_wall_timer(
        1000ms / init_params_.camera_fps,
        std::bind(&ZedCamPublisher::image_publisher_callback, this));
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ZedCamPublisher>());
    rclcpp::shutdown();
    return 0;
}
