#pragma once

#include <sl/Camera.hpp>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"

class ZedCamPublisher : public rclcpp::Node
{
public:
    ZedCamPublisher();

private:
    void image_publisher_callback();

    sl::Camera zed_cam_;
    sl::InitParameters init_params_;
    sl::RuntimeParameters runtime_params_;
    rclcpp::TimerBase::SharedPtr timer_cam_image_publisher_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher_right_cam_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher_left_cam_;
};

std::map<sl::VIDEO_SETTINGS, int> get_video_settings();
sl::RuntimeParameters get_runtime_params();
sl::ObjectDetectionParameters get_object_detection_params();
sl::PositionalTrackingParameters get_positional_tracking_params();
sl::InitParameters get_camera_init_params();
std::string interpret_error_code(sl::ERROR_CODE err);