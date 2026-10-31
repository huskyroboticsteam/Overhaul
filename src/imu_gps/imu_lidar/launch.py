from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package='imu_gps',
            executable='unitree_lidar_imu',
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
                'cloud_scan_num': 18,
                'range_min': 0.0,
                'range_max': 100.0,
                'imu_frame': 'unilidar_imu',
                'imu_topic': 'unilidar/imu',
                'quat_frame': 'unilidar_imu',
                'quat_topic': 'unilidar/quaternion',
            }],
        ),
    ])