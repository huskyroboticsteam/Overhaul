#include "unitree_lidar_ros2.h"

#include <chrono>
#include <functional>
#include <memory>
#include <stdexcept>

UnitreeLidarSDKNode::UnitreeLidarSDKNode(const rclcpp::NodeOptions &options)
  : Node("unitree_lidar_sdk_node", options)
{
  // Declare parameters (use port ttyACM0)
  declare_parameter<std::string>("port", "/dev/ttyACM0");
  declare_parameter<int>("baudrate", 4000000);
  declare_parameter<int>("cloud_scan_num", 18);
  declare_parameter<double>("range_max", 50.0);
  declare_parameter<double>("range_min", 0.0);

  declare_parameter<std::string>("imu_frame", "unilidar_imu");
  declare_parameter<std::string>("imu_topic", "unilidar/imu");

  // Read parameters
  port_ = get_parameter("port").as_string();
  baudrate_ = get_parameter("baudrate").as_int();
  cloud_scan_num_ = get_parameter("cloud_scan_num").as_int();
  range_max_ = get_parameter("range_max").as_double();
  range_min_ = get_parameter("range_min").as_double();

  imu_frame_ = get_parameter("imu_frame").as_string();
  imu_topic_ = get_parameter("imu_topic").as_string();

  // Initialize UnitreeLidarReader (SDK2)
  lsdk_ = unilidar_sdk2::createUnitreeLidarReader();
  if (lsdk_->initializeSerial(port_,
                              static_cast<uint32_t>(baudrate_),
                              static_cast<uint16_t>(cloud_scan_num_),
                              true,
                              static_cast<float>(range_min_),
                              static_cast<float>(range_max_)) != 0)
  {
    RCLCPP_ERROR(get_logger(), "Failed to open serial port %s", port_.c_str());
    throw std::runtime_error("Unitree lidar initialization failed");
  }

  // Create Publisher for IMU Data
  pub_imu_  = create_publisher<sensor_msgs::msg::Imu>(imu_topic_, 10);
  RCLCPP_INFO(this->get_logger(), "LIDAR IMU Data Publisher Created");

  timer_ = create_wall_timer(std::chrono::milliseconds(1),
                             std::bind(&UnitreeLidarSDKNode::timer_callback, this));
  RCLCPP_INFO(this->get_logger(), "Polling from LiDAR at 10 Hz");
}

// Shut down node when entering ctrl + c
UnitreeLidarSDKNode::~UnitreeLidarSDKNode()
{
  if (lsdk_ != nullptr)
  {
    lsdk_->closeSerial();  
    lsdk_ = nullptr;
    RCLCPP_INFO(this->get_logger(), "Unitree Lidar serial port closed successfully.");
  }
}



void UnitreeLidarSDKNode::timer_callback()
{
  // ensure received packets are IMU packets
  if (lsdk_->runParse() != LIDAR_IMU_DATA_PACKET_TYPE) return;

  // Poll and extract sensor data
  unilidar_sdk2::LidarImuData imu;
  if (!lsdk_->getImuData(imu)) return;

  // get timestamp of IMU packet
  const rclcpp::Time timestamp = this->now();

  // Create ROS Message
  sensor_msgs::msg::Imu imuMsg;
  imuMsg.header.frame_id = imu_frame_;
  imuMsg.header.stamp = timestamp;

  // Retrieve Quaternion Data from LiDAR
  imuMsg.orientation.x = imu.quaternion[0];
  imuMsg.orientation.y = imu.quaternion[1];
  imuMsg.orientation.z = imu.quaternion[2];
  imuMsg.orientation.w = imu.quaternion[3];

  // Retrieve Angular Velocity from LiDAR
  imuMsg.angular_velocity.x = imu.angular_velocity[0];
  imuMsg.angular_velocity.y = imu.angular_velocity[1];
  imuMsg.angular_velocity.z = imu.angular_velocity[2];

  // Retrieve Linear Acceleration from LiDAR
  imuMsg.linear_acceleration.x = imu.linear_acceleration[0];
  imuMsg.linear_acceleration.y = imu.linear_acceleration[1];
  imuMsg.linear_acceleration.z = imu.linear_acceleration[2];


  // Broadcast full IMU packet with timestamp to unilidar/imu topic
  pub_imu_->publish(imuMsg);
}

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<UnitreeLidarSDKNode>());
  rclcpp::shutdown();
  return 0;
}