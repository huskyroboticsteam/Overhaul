#!/usr/bin/env python3
"""Run manually on Linux with an already-created vcan0; never targets physical CAN."""
import os
from pathlib import Path
import socket
import struct
import subprocess
import time

import rclpy
from ament_index_python.packages import get_package_prefix
from can.msg import CANPacket


def main():
    os.environ['ROS_DOMAIN_ID'] = '189'
    os.environ['ROS_LOCALHOST_ONLY'] = '1'
    rclpy.init()
    node = rclpy.create_node('socketcan_smoke')
    pub = node.create_publisher(CANPacket, '/can_tx', 10)
    received = []
    node.create_subscription(CANPacket, '/can_rx', received.append, 100)
    raw = socket.socket(socket.AF_CAN, socket.SOCK_RAW, socket.CAN_RAW)
    raw.bind(('vcan0',))
    raw.setblocking(False)
    processes = []

    def pump_until(predicate, timeout=5):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            rclpy.spin_once(node, timeout_sec=0.01)
            if predicate():
                return
        raise AssertionError('SocketCAN smoke condition timed out')

    def settle():
        deadline = time.monotonic() + 0.15
        while time.monotonic() < deadline:
            rclpy.spin_once(node, timeout_sec=0.01)

    try:
        directory = Path(get_package_prefix('can')) / 'lib' / 'can'
        for executable in ('CANSender', 'CANReceiver'):
            processes.append(subprocess.Popen([
                str(directory / executable), '--ros-args', '-p', 'CAN_name:=vcan0']))
        pump_until(lambda: pub.get_subscription_count() == 1 and
                   node.count_publishers('/can_rx') == 1)

        msg = CANPacket()
        msg.device.device_uuid = 48
        msg.device.motor = True
        msg.sender_uuid = 1
        msg.command = 0x89
        msg.contents_length = 6
        msg.contents = [0, 0, 0x80, 0x3f, 0, 0]
        pub.publish(msg)
        frames = []

        def frame_arrived():
            try:
                frames.append(struct.unpack('=IB3x8s', raw.recv(16)))
            except BlockingIOError:
                pass
            return bool(frames)

        pump_until(frame_arrived)
        assert frames[-1] == (0x582, 8, bytes([0x89, 1, 0, 0, 0x80, 0x3f, 0, 0]))

        payload = bytes([0x0e, 48, 0, 0, 0, 0, 0x80, 0x3f])
        raw.send(struct.pack('=IB3x8s', 0x40a, 8, payload))
        pump_until(lambda: len(received) == 1)
        packet = received[-1]
        assert packet.device.device_uuid == 1 and packet.device.motor
        assert not packet.device.power and not packet.device.peripheral
        assert not packet.priority and packet.sender_uuid == 48
        assert packet.command == 0x0e and packet.contents_length == 6
        assert bytes(packet.contents) == payload[2:]

        received.clear()
        for can_id, length in ((0x40a, 1), (0x40a | socket.CAN_RTR_FLAG, 8),
                               (0x40a | socket.CAN_EFF_FLAG, 8)):
            raw.send(struct.pack('=IB3x8s', can_id, length, payload))
        settle()
        assert not received, 'Malformed/non-data frames must not reach can_rx'
        raw.send(struct.pack('=IB3x8s', 0x00f, 8, payload))
        pump_until(lambda: bool(received))
        assert received[-1].priority
        assert received[-1].device.motor and received[-1].device.power
        assert received[-1].device.peripheral

        msg.contents_length = 7
        pub.publish(msg)
        settle()
        try:
            raw.recv(16)
            raise AssertionError('Invalid outgoing length was transmitted')
        except BlockingIOError:
            pass
        assert all(p.poll() is None for p in processes)
        print('SocketCAN sender/receiver smoke passed')
    finally:
        for child in processes:
            child.terminate()
        for child in processes:
            child.wait(timeout=5)
        raw.close()
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
