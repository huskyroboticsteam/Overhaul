#!/usr/bin/env python3
import atexit
import time
import sys

import argparse
import math
import numpy as np

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Quaternion
from sensor_msgs.msg import NavSatFix

# This if is required because dronekit is not up to date for python 3.10
if sys.version_info.major == 3 and sys.version_info.minor >= 10:
    import collections
    setattr(collections, "MutableMapping", collections.abc.MutableMapping)
import dronekit

# Method epically stolen from ROS2 Humble Tutorial, see
# https://docs.ros.org/en/humble/Tutorials/Intermediate/Tf2/Writing-A-Tf2-Static-Broadcaster-Py.html
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

def debug_GPS_vars():
    args = get_args()
    print("Waiting for connection to {}...".format(args.port))
    ardupilot = dronekit.connect(args.port, wait_ready=True, baud=args.baud)
    print("Connection established to {}!".format(args.port))

    while True:
        time.sleep(1)
        lat = ardupilot.location.global_frame.lat
        lon = ardupilot.location.global_frame.lon
        alt = ardupilot.location.global_frame.alt
        roll = ardupilot.attitude.roll
        pitch = ardupilot.attitude.pitch
        yaw = ardupilot.heading
        print("lat: {}, lon: {}, alt {}, roll: {}, pitch: {}, yaw: {}"
              .format(lat, lon, alt, roll, pitch, yaw))


def get_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--port", default="/dev/ttyARDUPILOT",
                        help="Serial port connected to ardupilot")
    parser.add_argument("-hz", "--frequency", type=float,
                        default=10.0, help="Rate at which data is sent")
    parser.add_argument("-b", "--baud", default=115200,
                        type=int, help="Baud rate of serial connection")
    parser.add_argument("--debug", action="store_true",
                        help="Prints debug info to stdout")
    return parser.parse_args()


class IMUGPSPublisher(Node):
    def __init__(self):
        super().__init__('pixhawk_imu_gps_publisher')
        self.publisher_imu_ = self.create_publisher(Quaternion, 'pixhawk_imu', 10)
        self.publisher_gps_ = self.create_publisher(NavSatFix, 'pixhawk_gps', 10)
        timer_period = 0.1  # seconds
        self.timer = self.create_timer(timer_period, self.timer_callback)

        args = get_args()
        print("Waiting for ardupilot connection on port {}...".format(args.port))
        self.ardupilot = dronekit.connect(args.port, wait_ready=True, baud=args.baud)
        print("Connection established to ardupilot!")

        def cleanup():
            print("Exiting!")
            self.ardupilot.close()
        atexit.register(cleanup)
        self.ardupilot.parameters["COMPASS_ORIENT"] = 6


    def timer_callback(self):
        q = quaternion_from_euler(self.ardupilot.attitude.roll, self.ardupilot.attitude.pitch, self.ardupilot.attitude.yaw)
        msg_imu = Quaternion()
        msg_imu.x = q[0]
        msg_imu.y = q[1]
        msg_imu.z = q[2]
        msg_imu.w = q[3]

        msg_gps = NavSatFix()
        msg_gps.latitude = self.ardupilot.location.global_frame.lat
        msg_gps.longitude = self.ardupilot.location.global_frame.lon
        msg_gps.altitude = self.ardupilot.location.global_frame.alt

        self.publisher_imu_.publish(msg_imu)
        self.publisher_gps_.publish(msg_gps)

        self.get_logger().info('Publishing Pixhawk IMU Data as Quaternion: x: "%s" y: "%s" z: "%s" w: "%s"' % (msg_imu.x, msg_imu.y, msg_imu.z, msg_imu.w))
        self.get_logger().info('Publishing Pixhawk GPS Data: Latitude: "%s" Longitude: "%s" Altitude: "%s"' % (msg_gps.latitude, msg_gps.longitude, msg_gps.altitude))
        if (self.ardupilot._last_heartbeat > self.ardupilot._heartbeat_warning):
            self.get_logger().warn("No heartbeat from Pixhawk for {} seconds".format(self.ardupilot._last_heartbeat))
        if (self.ardupilot._last_heartbeat > self.ardupilot._heartbeat_error):
            self.get_logger().error("No heartbeat from Pixhawk for {} seconds".format(self.ardupilot._last_heartbeat))
        if (msg_gps.latitude == 0 or msg_gps.longitude == 0 or msg_gps.altitude == 0):
            self.get_logger().warn("GPS data from the Pixhawk is zero, check Pixhawk connection")

def main(args=None):
    rclpy.init(args=args)
    publisher = IMUGPSPublisher()
    rclpy.spin(publisher)
    publisher.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()