#!/usr/bin/env python3
import atexit
import sys
import argparse
import math
import numpy as np

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import QuaternionStamped
from sensor_msgs.msg import NavSatFix

# This if is required because dronekit is not up to date for python 3.10
if sys.version_info.major == 3 and sys.version_info.minor >= 10:
    import collections
    setattr(collections, "MutableMapping", collections.abc.MutableMapping)
import dronekit

# Method epically stolen from ROS2 Humble Tutorial, see
# https://docs.ros.org/en/humble/Tutorials/Intermediate/Tf2/Writing-A-Tf2-Static-Broadcaster-Py.html
# Converts Euler angles (in radians) (roll, pitch, yaw) to a quaternion.
def quaternion_from_euler(ai, aj, ak):
    ai /= 2.0
    aj /= 2.0
    ak /= 2.0
    ci = math.cos(ai)
    si = math.sin(ai)
    cj = math.cos(aj)
    sj = math.sin(aj)
    ck = math.cos(ak)
    sk = math.sin(ak)
    cc = ci*ck
    cs = ci*sk
    sc = si*ck
    ss = si*sk

    q = np.empty((4, ))
    q[0] = cj*sc - sj*cs
    q[1] = cj*ss + sj*cc
    q[2] = cj*cs - sj*sc
    q[3] = cj*cc + sj*ss

    return q

# Gets the arguements from the command line
def get_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--port", default="/dev/ttyARDUPILOT",
                        help="Serial port connected to ardupilot")
    parser.add_argument("-hz", "--frequency", type=float,
                        default=10.0, help="Rate at which data is sent")
    parser.add_argument("-b", "--baud", default=115200,
                        type=int, help="Baud rate of serial connection")
    return parser.parse_args()

# Creates a new Node that publishes IMU and GPS data from the Pixhawk.
class IMUGPSPublisher(Node):

    # Creates the 2 publishers that output QuaternionStamped and NavSatFix messages 
    # at the specified frequency (default 10 Hz)
    # Also connects to the Pixhawk. Remember to run as root
    def __init__(self):
        super().__init__('pixhawk_imu_gps_publisher')
        args = get_args()
        self.publisher_imu_ = self.create_publisher(QuaternionStamped, 'pixhawk_imu', int(args.frequency))
        self.publisher_gps_ = self.create_publisher(NavSatFix, 'pixhawk_gps', int(args.frequency))
        timer_period = 1.0 / args.frequency  # seconds
        self.timer = self.create_timer(timer_period, self.timer_callback)

        print("Waiting for ardupilot connection on port {}...".format(args.port))
        self.ardupilot = dronekit.connect(args.port, wait_ready=True, baud=args.baud)
        print("Connection established to ardupilot!")

        def cleanup():
            print("Exiting!")
            self.ardupilot.close()
        atexit.register(cleanup)
        self.ardupilot.parameters["COMPASS_ORIENT"] = 6

    # Callback function for the timer, publishes IMU and GPS data at the specified frequency
    # Also logs the published data and checks for heartbeat and GPS validity
    def timer_callback(self):
        q = quaternion_from_euler(self.ardupilot.attitude.roll, self.ardupilot.attitude.pitch, self.ardupilot.attitude.yaw)
        msg_imu = QuaternionStamped()
        msg_imu.header.stamp = self.get_clock().now().to_msg()
        msg_imu.quaternion.x = q[0]
        msg_imu.quaternion.y = q[1]
        msg_imu.quaternion.z = q[2]
        msg_imu.quaternion.w = q[3]
        msg_imu.header.frame_id = "pixhawk_imu"

        msg_gps = NavSatFix()
        msg_gps.header.stamp = self.get_clock().now().to_msg()
        msg_gps.latitude = self.ardupilot.location.global_frame.lat
        msg_gps.longitude = self.ardupilot.location.global_frame.lon
        msg_gps.altitude = self.ardupilot.location.global_frame.alt
        msg_gps.header.frame_id = "pixhawk_gps"

        self.publisher_imu_.publish(msg_imu)
        self.publisher_gps_.publish(msg_gps)

        self.get_logger().info('Publishing Pixhawk IMU Data as Quaternion: x: "%s" y: "%s" z: "%s" w: "%s"' % (msg_imu.quaternion.x, msg_imu.quaternion.y, msg_imu.quaternion.z, msg_imu.quaternion.w))
        self.get_logger().info('Publishing Pixhawk GPS Data: Latitude: "%s" Longitude: "%s" Altitude: "%s"' % (msg_gps.latitude, msg_gps.longitude, msg_gps.altitude))
        if (self.ardupilot._last_heartbeat > self.ardupilot._heartbeat_warning):
            self.get_logger().warn("No heartbeat from Pixhawk for {} seconds".format(self.ardupilot._last_heartbeat))
        if (self.ardupilot._last_heartbeat > self.ardupilot._heartbeat_error):
            self.get_logger().error("No heartbeat from Pixhawk for {} seconds".format(self.ardupilot._last_heartbeat))
        if (msg_gps.latitude == 0 or msg_gps.longitude == 0 or msg_gps.altitude == 0):
            self.get_logger().warn("GPS data from the Pixhawk is zero, check Pixhawk connection")

# Runs the code
def main(args=None):
    rclpy.init(args=args)
    publisher = IMUGPSPublisher()
    rclpy.spin(publisher)
    publisher.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()