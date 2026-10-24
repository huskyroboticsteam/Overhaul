#!/usr/bin/env python3
import atexit
import time
import sys
import asyncio
import websockets
import json
import argparse
import math

import rclpy
from rclpy.node import Node
from std_msgs.msg import String
from geometry_msgs.msg import Vector3
from imu_gps.msg import GPSData, IMUData

# This if is required because dronekit is not up to date for python 3.10
if sys.version_info.major == 3 and sys.version_info.minor >= 10:
    import collections
    setattr(collections, "MutableMapping", collections.abc.MutableMapping)
import dronekit


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
        self.publisher_imu_ = self.create_publisher(IMUData, 'pixhawk_imu', 10)
        self.publisher_gps_ = self.create_publisher(GPSData, 'pixhawk_gps', 10)
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
        msg_imu = IMUData(pitch=self.ardupilot.attitude.pitch, yaw=self.ardupilot.attitude.yaw, roll=self.ardupilot.attitude.roll)
        msg_gps = GPSData(latitute=self.ardupilot.location.global_frame.lat, longitude=self.ardupilot.location.global_frame.lon, altitude=self.ardupilot.location.global_frame.alt)
        self.publisher_imu_.publish(msg_imu)
        self.publisher_gps_.publish(msg_gps)
        self.get_logger().info('Publishing Pixhawk IMU Data: Pitch: "%s" Yaw: "%s" Roll: "%s"' % (msg_imu.pitch, msg_imu.yaw, msg_imu.roll))
        self.get_logger().info('Publishing Pixhawk GPS Data: Latitude: "%s" Longitude: "%s" Altitude: "%s"' % (msg_gps.latitute, msg_gps.longitude, msg_gps.altitude))
        if (self.ardupilot._last_heartbeat > self.ardupilot._heartbeat_warning):
            self.get_logger().warn("No heartbeat from Pixhawk for {} seconds".format(self.ardupilot._last_heartbeat))
        if (self.ardupilot._last_heartbeat > self.ardupilot._heartbeat_error):
            self.get_logger().error("No heartbeat from Pixhawk for {} seconds".format(self.ardupilot._last_heartbeat))
        if (msg_gps.latitute == 0 or msg_gps.longitude == 0 or msg_gps.altitude == 0):
            self.get_logger().warn("GPS data from the Pixhawk is zero, check Pixhawk connection")

def main(args=None):
    rclpy.init(args=args)
    publisher = IMUGPSPublisher()
    rclpy.spin(publisher)
    publisher.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()