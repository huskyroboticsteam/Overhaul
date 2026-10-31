#include "unitree_lidar_ros2.h"

#include <chrono>
#include <functional>
#include <memory>
#include <stdexcept>

UnitreeLidarSDKNode::UnitreeLidarSDKNode(const rclcpp::NodeOptions &options)
  : Node("unitree_lidar_sdk_node", options)
{
  // declare parameters
  declare_parameter<std::string>("port", "/dev/ttyACM0");
  declare_parameter<int>("baudrate", 4000000);
  declare_parameter<int>("cloud_scan_num", 18);
  declare_parameter<double>("range_max", 50.0);
  declare_parameter<double>("range_min", 0.0);

  declare_parameter<std::string>("imu_frame", "unilidar_imu");
  declare_parameter<std::string>("imu_topic", "unilidar/imu");

  declare_parameter<std::string>("quat_frame", "unilidar_imu");
  declare_parameter<std::string>("quat_topic", "unilidar/quaternion");

  // read parameters
  port_ = get_parameter("port").as_string();
  baudrate_ = get_parameter("baudrate").as_int();
  cloud_scan_num_ = get_parameter("cloud_scan_num").as_int();
  range_max_ = get_parameter("range_max").as_double();
  range_min_ = get_parameter("range_min").as_double();

  imu_frame_ = get_parameter("imu_frame").as_string();
  imu_topic_ = get_parameter("imu_topic").as_string();

  quat_frame_ = get_parameter("quat_frame").as_string();
  quat_topic_ = get_parameter("quat_topic").as_string();

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

  // ROS2
  pub_imu_  = create_publisher<sensor_msgs::msg::Imu>(imu_topic_, 10);
  pub_quat_ = create_publisher<geometry_msgs::msg::QuaternionStamped>(quat_topic_, 10);
  timer_ = create_wall_timer(std::chrono::milliseconds(1),
                             std::bind(&UnitreeLidarSDKNode::timer_callback, this));
}

UnitreeLidarSDKNode::~UnitreeLidarSDKNode()
{
  if (lsdk_ != nullptr)
  {
    lsdk_->closeSerial();   // SDK owns the object; don't delete it
    lsdk_ = nullptr;
  }
}

void UnitreeLidarSDKNode::timer_callback()
{
  if (lsdk_->runParse() != LIDAR_IMU_DATA_PACKET_TYPE) return;

  unilidar_sdk2::LidarImuData imu;
  if (!lsdk_->getImuData(imu)) return;

  // Use the ROS clock; the lidar's hardware clock is not epoch-synced by default.
  const rclcpp::Time timestamp = this->now();

  // Orientation goes to Quaternion Topic
  geometry_msgs::msg::QuaternionStamped quatMsg;
  quatMsg.header.frame_id = quat_frame_;
  quatMsg.header.stamp = timestamp;
  quatMsg.quaternion.x = imu.quaternion[0];
  quatMsg.quaternion.y = imu.quaternion[1];
  quatMsg.quaternion.z = imu.quaternion[2];
  quatMsg.quaternion.w = imu.quaternion[3];

  // Linear Acceleration + Angular Velocity goes to Imu topic
  sensor_msgs::msg::Imu imuMsg;
  imuMsg.header.frame_id = imu_frame_;
  imuMsg.header.stamp = timestamp;
  imuMsg.orientation_covariance[0] = -1.0;   // orientation intentionally not provided here
  imuMsg.angular_velocity.x = imu.angular_velocity[0];
  imuMsg.angular_velocity.y = imu.angular_velocity[1];
  imuMsg.angular_velocity.z = imu.angular_velocity[2];
  imuMsg.linear_acceleration.x = imu.linear_acceleration[0];
  imuMsg.linear_acceleration.y = imu.linear_acceleration[1];
  imuMsg.linear_acceleration.z = imu.linear_acceleration[2];

  pub_quat_->publish(quatMsg);
  pub_imu_->publish(imuMsg);
}

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<UnitreeLidarSDKNode>());
  rclcpp::shutdown();
  return 0;
}