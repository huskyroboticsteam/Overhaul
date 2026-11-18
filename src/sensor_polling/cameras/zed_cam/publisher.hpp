#pragma once

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"

class ZedCamPublisher : public rclcpp::Node
{
public:
    ZedCamPublisher();

private:
    void right_cam_callback();

    rclcpp::TimerBase::SharedPtr timer_right_cam_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr
        publisher_right_cam_;
};