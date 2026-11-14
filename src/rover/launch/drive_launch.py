"""One-wheel bring-up. Default backend is fake; hardware requires explicit configuration."""
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution, PythonExpression
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    backend = LaunchConfiguration('backend')
    config = LaunchConfiguration('config')
    hardware = IfCondition(PythonExpression(["'", backend, "' == 'can'"]))
    return LaunchDescription([
        DeclareLaunchArgument('backend', default_value='fake', choices=['fake', 'can', 'sim']),
        DeclareLaunchArgument('input_mode', default_value='twist', choices=['twist', 'wheel']),
        DeclareLaunchArgument('config', default_value=PathJoinSubstitution([
            get_package_share_directory('rover'), 'config', 'motors.yaml'])),
        DeclareLaunchArgument('can_interface', default_value='can0'),
        DeclareLaunchArgument('enable_motor', default_value='false'),
        DeclareLaunchArgument('motor_uuid', default_value='-1'),
        DeclareLaunchArgument('gear_ratio', default_value='0.0'),
        Node(package='rover', executable='Drive', name='drive', parameters=[config],
             condition=IfCondition(PythonExpression([
                 "'", LaunchConfiguration('input_mode'), "' == 'twist'"])), output='screen'),
        Node(package='rover', executable='WheelMotor', name='front_left_motor',
             parameters=[config, {
                 'backend': backend,
                 'enable_motor': ParameterValue(LaunchConfiguration('enable_motor'), value_type=bool),
                 'motor_uuid': ParameterValue(LaunchConfiguration('motor_uuid'), value_type=int),
                 'gear_ratio': ParameterValue(LaunchConfiguration('gear_ratio'), value_type=float),
             }], output='screen'),
        Node(package='can', executable='CANSender', parameters=[{
            'CAN_name': LaunchConfiguration('can_interface')}], condition=hardware, output='screen'),
        Node(package='can', executable='CANReceiver', parameters=[{
            'CAN_name': LaunchConfiguration('can_interface')}], condition=hardware, output='screen'),
    ])
