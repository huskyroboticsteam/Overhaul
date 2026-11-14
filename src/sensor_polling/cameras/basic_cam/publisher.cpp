#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "sensor_msgs/image_encodings.hpp"


#include <opencv2/opencv.hpp>
#include "cv_bridge/cv_bridge.h"


using namespace std::chrono_literals;

class BasicCamPublisher : public rclcpp::Node
{
    public:
        BasicCamPublisher(const std::string &node_name, const int hz, const std::string &camera_dev_id)
        : Node(node_name), count_(0)
        {
            publisher_ = this->create_publisher<sensor_msgs::msg::Image>(std::string("/camera/basic_cam/") + node_name, 10);
            cap_ = cv::VideoCapture();
            frame_id_ = node_name;

            while (rclcpp::ok() && !cap_.open(camera_dev_id, cv::CAP_V4L2)) {
                RCLCPP_ERROR(get_logger(), "Error: Could not open the webcam.");
                rclcpp::sleep_for(1s);
            }
	    timer_ = this->create_wall_timer(
            1000ms / hz, std::bind(&BasicCamPublisher::timer_callback, this));
        }

    private:
        void timer_callback()
        {
	    if (!cap_.isOpened()) {
	        return;
	    }
            cv::Mat frame;
            if (!cap_.read(frame) || frame.empty() || frame.rows <= 0 || frame.cols <= 0) {
                RCLCPP_WARN(get_logger(), "WARNING: Did not get a frame from the camera.");
                return;
            }
            std_msgs::msg::Header header;
            header.stamp = this->now();
            header.frame_id = frame_id_;
            sensor_msgs::msg::Image::SharedPtr message = cv_bridge::CvImage(header, sensor_msgs::image_encodings::BGR8, frame).toImageMsg();
            publisher_->publish(*message);
        }
        
        rclcpp::TimerBase::SharedPtr timer_;
        rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher_;
        size_t count_;
        cv::VideoCapture cap_;
        std::string frame_id_;
};

int main(int argc, char * argv[])
{
    if (argc < 4) {
        
        std::cerr << "Usage: basic_cam_publisher <node_name> <hz> <camera_dev_id>" << std::endl;
        std::cerr << "Ex: basic_cam_publisher wrist 50 /dev/video0" << std::endl;
        RCLCPP_ERROR(rclcpp::get_logger("basic_cam"), "Incorrect params passed into basic camera publisher");
        return 1;
    }
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<BasicCamPublisher>(argv[1], std::stoi(argv[2]), argv[3]));
    rclcpp::shutdown();
    return 0;
}
