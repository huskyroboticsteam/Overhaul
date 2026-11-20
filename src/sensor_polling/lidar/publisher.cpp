#include "unitree_lidar_ros2.h"

#include <chrono>
#include <functional>
#include <memory>
#include <stdexcept>

UnitreeLidarSDKNode::UnitreeLidarSDKNode(const rclcpp::NodeOptions &options)
  : Node("unitree_lidar_sdk_node", options)
{
  // Declare parameters (use port ttyACM0)
  declare_parameter<std::string>("port", "/dev/unitree_lidar");
  declare_parameter<int>("baudrate", 4000000);
  declare_parameter<int>("cloud_scan_num", 18);
  declare_parameter<double>("range_max", 50.0);
  declare_parameter<double>("range_min", 0.0);
  declare_parameter<std::string>("imu_frame", "unilidar_imu");
  declare_parameter<std::string>("imu_topic", "/imu/lidar");

  declare_parameter<std::string>("cloud_frame", "unilidar_lidar");
  declare_parameter<std::string>("cloud_topic", "unilidar/cloud");

  // Read parameters
  port_ = get_parameter("port").as_string();
  baudrate_ = get_parameter("baudrate").as_int();

  cloud_scan_num_ = get_parameter("cloud_scan_num").as_int();
  range_max_ = get_parameter("range_max").as_double();
  range_min_ = get_parameter("range_min").as_double();

  imu_frame_ = get_parameter("imu_frame").as_string();
  imu_topic_ = get_parameter("imu_topic").as_string();

  cloud_frame_ = get_parameter("cloud_frame").as_string();
  cloud_topic_ = get_parameter("cloud_topic").as_string();

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

  pub_cloud_ = create_publisher<sensor_msgs::msg::PointCloud2>(cloud_topic_, 10);
  RCLCPP_INFO(this->get_logger(), "LIDAR Point Cloud Publisher Created");
  cloudOut_ = std::make_shared<pcl::PointCloud<PointType>>();

  timer_ = create_wall_timer(std::chrono::milliseconds(1),
                             std::bind(&UnitreeLidarSDKNode::timer_callback, this));
  RCLCPP_INFO(this->get_logger(), "Polling from LiDAR at 1000 Hz, publishing at 500 Hz");
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

// Poll, receive, and publish data at 500 Hz
void UnitreeLidarSDKNode::timer_callback()
{
  int result;
  while ((result = lsdk_->runParse()) != 0)
  {
    // check if an IMU data packet is being received
    if (result == LIDAR_IMU_DATA_PACKET_TYPE)
    {
      unilidar_sdk2::LidarImuData imu;
      if (!lsdk_->getImuData(imu)) continue;


      // define IMU message
      sensor_msgs::msg::Imu imuMsg;
      imuMsg.header.frame_id = imu_frame_;
      imuMsg.header.stamp = this->now();

      // retrieve IMU 3D orientation
      imuMsg.orientation.x = imu.quaternion[0];
      imuMsg.orientation.y = imu.quaternion[1];
      imuMsg.orientation.z = imu.quaternion[2];
      imuMsg.orientation.w = imu.quaternion[3];
      imuMsg.orientation_covariance = {8.543068027834089e-07, 2.0119617120236034e-08, 1.2355307220230947e-07,
                                       2.0119617120236034e-08, 2.612990272942886e-06, 1.4687900427637633e-07,
                                       1.2355307220230947e-07, 1.4687900427637633e-07, 1.0493471902336669e-06};

      // retrieve IMU rotational speed                                 
      imuMsg.angular_velocity.x = imu.angular_velocity[0];
      imuMsg.angular_velocity.y = imu.angular_velocity[1];
      imuMsg.angular_velocity.z = imu.angular_velocity[2];
      imuMsg.angular_velocity_covariance = {0.0003557801624545512, 2.276833304388468e-05, -9.766761338406529e-05,
                                            2.276833304388468e-05, 0.0004791685591795729, 4.155375687924402e-05,
                                            -9.766761338406529e-05, 4.155375687924402e-05, 0.00045224778701255866};

      // retrieve accelerometer data
      imuMsg.linear_acceleration.x = imu.linear_acceleration[0];
      imuMsg.linear_acceleration.y = imu.linear_acceleration[1];
      imuMsg.linear_acceleration.z = imu.linear_acceleration[2];
      imuMsg.linear_acceleration_covariance = {0.10027424272803934, -0.0014685746365046056, -0.03839344188748068,
                                               -0.0014685746365046056, 0.036102655563778126, 0.030189153318523607,
                                               -0.03839344188748068, 0.030189153318523607, 0.311645705112446};

      // publish to IMU topic
      pub_imu_->publish(imuMsg);
    }

    // Check if receiving a LIDAR Point CLoud Data Point
    else if (result == LIDAR_POINT_DATA_PACKET_TYPE)
    {
      PointCloudUnitree cloud;
      if (!lsdk_->getPointCloud(cloud)) continue;

      transformUnitreeCloudToPCL(cloud, cloudOut_);

      sensor_msgs::msg::PointCloud2 cloud_msg;
      pcl::toROSMsg(*cloudOut_, cloud_msg);
      cloud_msg.header.frame_id = cloud_frame_;
      cloud_msg.header.stamp = rclcpp::Time(
          static_cast<int32_t>(cloud.stamp),
          static_cast<uint32_t>((cloud.stamp - static_cast<int32_t>(cloud.stamp)) * 1e9));
      pub_cloud_->publish(cloud_msg);
    }
  }
}

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<UnitreeLidarSDKNode>());
  rclcpp::shutdown();
  return 0;
}