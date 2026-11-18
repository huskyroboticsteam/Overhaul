#include "publisher.hpp";

void ZedCamPublisher::right_cam_callback()
{
    sensor_msgs::msg::Image msg;
    publisher_right_cam_->publish(msg);
}