import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
  # Run unitree lidar
  node1 = Node(
    package='sensor_polling',
    executable='unitree_lidar',
    name='unitree_lidar_sdk_node',
    output='screen',
    parameters=[{
        'initialize_type': 1,        
        'work_mode': 0,
        'port': '/dev/ttyACM0',
        'baudrate': 4000000,
        'lidar_port': 6101,
        'lidar_ip': '192.168.1.62',
        'local_port': 6201,
        'local_ip': '192.168.1.2',
        'cloud_frame': "unilidar_lidar",
        'cloud_topic': "unilidar/cloud",
        'cloud_scan_num': 18,
        'range_min': 0.0,
        'range_max': 100.0,
        'imu_frame': 'unilidar_imu',
        'imu_topic': 'unilidar/imu',
        }],
    )

  # Rviz
  
  rviz_config_file = os.path.join(
    get_package_share_directory('sensor_polling'), 'rviz', 'view.rviz')
  print("rviz_config_file = " + rviz_config_file)
  rviz_node = Node(
    package='rviz2',
    executable='rviz2',
    name='rviz2',
    arguments=['-d', rviz_config_file],
    output='log'
  )
  return LaunchDescription([node1, rviz_node])



