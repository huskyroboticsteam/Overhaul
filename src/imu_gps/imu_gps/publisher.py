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
        self.publisher_imu_ = self.create_publisher(Vector3, 'pixhawk_imu', 10)
        self.publisher_gps_ = self.create_publisher(Vector3, 'pixhawk_gps', 10)
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
        msg_imu = Vector3(x=self.ardupilot.attitude.pitch, y=self.ardupilot.attitude.yaw, z=self.ardupilot.attitude.roll)
        msg_gps = Vector3(x=self.ardupilot.location.global_frame.lat, y=self.ardupilot.location.global_frame.lon, z=self.ardupilot.location.global_frame.alt)
        self.publisher_imu_.publish(msg_imu)
        self.publisher_gps_.publish(msg_gps)
        self.get_logger().info('Publishing: "%s"' % msg_imu.x)
        self.get_logger().info('Publishing: "%s"' % msg_gps.x)

def main(args=None):
    print("hi")
    rclpy.init(args=args)
    publisher = IMUGPSPublisher()
    rclpy.spin(publisher)
    publisher.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()