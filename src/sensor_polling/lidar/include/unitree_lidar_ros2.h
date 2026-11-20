/**********************************************************************
 Copyright (c) 2020-2024, Unitree Robotics.Co.Ltd. All rights reserved.
***********************************************************************/

#pragma once

#define BOOST_BIND_NO_PLACEHOLDERS

#include <memory>
#include "iostream"
#include <string>
#include <stdio.h>
#include <iterator>
#include <algorithm>
#include <chrono>

#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl_conversions/pcl_conversions.h>

#include "sensor_msgs/msg/point_cloud2.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "std_msgs/msg/header.hpp"

#include "rclcpp/rclcpp.hpp"
#include "rclcpp/time.hpp"

#include "tf2_ros/transform_broadcaster.h"
#include "tf2/LinearMath/Quaternion.h"
#include "geometry_msgs/msg/transform_stamped.hpp"

// SDK
#include "unitree_lidar_sdk_pcl.h"

using std::placeholders::_1;

class UnitreeLidarSDKNode : public rclcpp::Node
{
public:
    explicit UnitreeLidarSDKNode(const rclcpp::NodeOptions &options = rclcpp::NodeOptions());

    ~UnitreeLidarSDKNode();

    void timer_callback();

protected:
    // ROS
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr pub_cloud_;
    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr pub_imu_;
    rclcpp::TimerBase::SharedPtr timer_;
    std::shared_ptr<tf2_ros::TransformBroadcaster> broadcaster_;

    // Unitree Lidar Reader
    UnitreeLidarReader *lsdk_;

    // Config params
    int work_mode_;
    int initialize_type_;
    int local_port_;
    std::string local_ip_;
    int lidar_port_;
    std::string lidar_ip_;
    std::string port_;
    int baudrate_;
    
    int cloud_scan_num_;
    bool use_system_timestamp_;
    double range_min_;
    double range_max_;

    std::string cloud_frame_;
    std::string cloud_topic_;

    std::string imu_frame_;
    std::string imu_topic_;

    pcl::PointCloud<PointType>::Ptr cloudOut_;
};