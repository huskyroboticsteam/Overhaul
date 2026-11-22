#include "publisher.hpp"

void ZedCamPublisher::image_publisher_callback()
{
    sensor_msgs::msg::Image msg;
    publisher_right_cam_->publish(msg);
}
