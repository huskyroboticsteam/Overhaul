"""ROS integration tests with real nodes and an in-process CAN26 translator double."""
import contextlib
import math
import os
from pathlib import Path
import struct
import subprocess
import tempfile
import time

import pytest
import rclpy
from ament_index_python.packages import get_package_prefix
from can.msg import CANPacket
from drive_interfaces.msg import MotorStatus, WheelCommand
from geometry_msgs.msg import Twist
from std_msgs.msg import Float64


@pytest.fixture
def ros_node():
    os.environ['ROS_DOMAIN_ID'] = str(70 + os.getpid() % 100)
    os.environ['ROS_LOCALHOST_ONLY'] = '1'
    rclpy.init()
    node = rclpy.create_node('drive_test')
    yield node
    node.destroy_node()
    rclpy.shutdown()


@contextlib.contextmanager
def process(executable, *parameters):
    binary = Path(get_package_prefix('rover')) / 'lib' / 'rover' / executable
    args = [str(binary), '--ros-args']
    for parameter in parameters:
        args += ['-p', parameter]
    with tempfile.TemporaryFile(mode='w+') as log:
        child = subprocess.Popen(args, stdout=log, stderr=log)
        try:
            yield child
        except BaseException:
            log.seek(0)
            print(log.read())
            raise
        finally:
            child.terminate()
            try:
                child.wait(timeout=5)
            except subprocess.TimeoutExpired:
                child.kill()
                child.wait()


def wait_for(node, predicate, timeout=8, publish=None):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if publish:
            publish()
        rclpy.spin_once(node, timeout_sec=0.03)
        if predicate():
            return
    raise AssertionError('Timed out waiting for ROS condition')


def statuses(node):
    values = []
    node.create_subscription(MotorStatus, '/Drive/FrontLeft/status', values.append, 100)
    return values


def test_twist_fake_forward_reverse_zero_and_expiry(ros_node):
    values = statuses(ros_node)
    pub = ros_node.create_publisher(Twist, '/DriveRequest', 1)
    with process('Drive'), process('WheelMotor'):
        wait_for(ros_node, lambda: values and pub.get_subscription_count() > 0)
        for velocity in (0.3, -0.15, 0.0):
            values.clear()
            command = Twist()
            command.linear.x = velocity
            wait_for(ros_node, lambda: any(
                s.command_result == MotorStatus.VERIFIED and s.feedback_valid and
                math.isclose(s.measured_velocity, velocity / 0.15, abs_tol=1e-6)
                for s in values), publish=lambda: pub.publish(command))
        wait_for(ros_node, lambda: values[-1].command_result == MotorStatus.EXPIRED)
        assert values[-1].measured_velocity == 0


def test_direct_wheel_rejects_nonfinite_input(ros_node):
    values = statuses(ros_node)
    pub = ros_node.create_publisher(WheelCommand, '/Drive/FrontLeft', 1)
    with process('WheelMotor'):
        wait_for(ros_node, lambda: values and pub.get_subscription_count() > 0)
        command = WheelCommand(velocity=float('nan'), steering_angle=0.0)
        wait_for(ros_node, lambda: values[-1].command_result == MotorStatus.FAILED,
                 publish=lambda: pub.publish(command))
        assert 'Invalid command' in values[-1].fault


def test_sim_adapter_uses_measured_feedback(ros_node):
    values = statuses(ros_node)
    outputs = []
    pub = ros_node.create_publisher(WheelCommand, '/Drive/FrontLeft', 1)
    feedback = ros_node.create_publisher(Float64, '/Direction/FrontLeft/velocity', 1)
    ros_node.create_subscription(Twist, '/Direction/FrontLeft', outputs.append, 10)
    with process('WheelMotor', 'backend:=sim'):
        wait_for(ros_node, lambda: values and pub.get_subscription_count() > 0)
        command = WheelCommand(velocity=2.0, steering_angle=0.4)
        wait_for(ros_node, lambda: any(math.isclose(m.linear.x, 0.3) and
                 math.isclose(m.angular.x, 0.4) for m in outputs),
                 publish=lambda: pub.publish(command))
        assert not any(s.command_result == MotorStatus.VERIFIED for s in values)

        def send():
            pub.publish(command)
            feedback.publish(Float64(data=2.0))

        wait_for(ros_node, lambda: values[-1].command_result == MotorStatus.VERIFIED,
                 publish=send)
        assert values[-1].feedback_valid and values[-1].measured_velocity == 2.0


class Translator:
    def __init__(self, node, reply_to_reads=True, reject_velocity=False):
        self.pub = node.create_publisher(CANPacket, '/can_rx', 20)
        self.sub = node.create_subscription(CANPacket, '/can_tx', self.receive, 100)
        self.packets = []
        self.rev_s = 0.0
        self.reply_to_reads = reply_to_reads
        self.reject_velocity = reject_velocity

    def reply(self, command, payload):
        msg = CANPacket()
        msg.device.device_uuid = 1
        msg.sender_uuid = 48
        msg.command = command
        msg.contents_length = len(payload)
        msg.contents = list(payload) + [0] * (6 - len(payload))
        self.pub.publish(msg)

    def receive(self, packet):
        self.packets.append(packet)
        command = packet.command & 0x7f
        if command in (0x07, 0x11):
            self.reply(0x01, [0, command])
        elif command == 0x09:
            self.rev_s = struct.unpack('<f', bytes(packet.contents[:4]))[0]
            if self.reject_velocity and self.rev_s != 0:
                self.reply(0x01, [1, command])
            # Intentionally omit velocity ACK: readback must still verify motion.
        elif command == 0x0d and self.reply_to_reads:
            velocity = struct.pack('<f', self.rev_s)[1:]
            self.reply(0x0e, bytes(3) + velocity)


CAN_PARAMS = ('backend:=can', 'enable_motor:=true', 'motor_uuid:=48',
              'gear_ratio:=2.0', 'response_timeout:=0.8', 'feedback_timeout:=1.5',
              'command_timeout:=2.0', 'settle_timeout:=2.0')


def test_can_initialization_signed_commands_and_readback_without_ack(ros_node):
    translator = Translator(ros_node)
    values = statuses(ros_node)
    pub = ros_node.create_publisher(WheelCommand, '/Drive/FrontLeft', 1)
    with process('WheelMotor', *CAN_PARAMS):
        wait_for(ros_node, lambda: values and values[-1].ready)
        init = [p.command & 0x7f for p in translator.packets]
        assert init.index(0x07) < init.index(0x11)
        for velocity in (2.0, -3.0, 0.0):
            values.clear()
            command = WheelCommand(velocity=velocity, steering_angle=0.0)
            wait_for(ros_node, lambda: any(
                s.command_result == MotorStatus.VERIFIED and s.feedback_valid and
                math.isclose(s.measured_velocity, velocity, abs_tol=0.01)
                for s in values), publish=lambda: pub.publish(command))
        packets = [p for p in translator.packets if p.command == 0x89]
        assert packets
        assert all(p.device.device_uuid == 48 and p.device.motor and
                   p.sender_uuid == 1 and p.contents_length == 6 for p in packets)
        decoded = [struct.unpack('<f', bytes(p.contents[:4]))[0] for p in packets]
        assert any(math.isclose(v, 2 / math.pi, rel_tol=1e-6) for v in decoded)
        assert any(math.isclose(v, -3 / math.pi, rel_tol=1e-6) for v in decoded)


def test_can_missing_feedback_is_not_confirmed_by_ack(ros_node):
    translator = Translator(ros_node, reply_to_reads=False)
    values = statuses(ros_node)
    pub = ros_node.create_publisher(WheelCommand, '/Drive/FrontLeft', 1)
    with process('WheelMotor', *CAN_PARAMS):
        wait_for(ros_node, lambda: values and values[-1].ready)
        command = WheelCommand(velocity=2.0, steering_angle=0.0)

        def send():
            pub.publish(command)
            translator.reply(0x01, [0, 0x09])

        wait_for(ros_node, lambda: values[-1].command_result == MotorStatus.FAILED,
                 publish=send)
        assert 'timed out' in values[-1].fault
        assert not any(s.command_result == MotorStatus.VERIFIED for s in values)
        wait_for(ros_node, lambda: translator.rev_s == 0)


def test_can_rejected_velocity_stops_motor(ros_node):
    translator = Translator(ros_node, reject_velocity=True)
    values = statuses(ros_node)
    pub = ros_node.create_publisher(WheelCommand, '/Drive/FrontLeft', 1)
    with process('WheelMotor', *CAN_PARAMS):
        wait_for(ros_node, lambda: values and values[-1].ready)
        command = WheelCommand(velocity=2.0, steering_angle=0.0)
        wait_for(ros_node, lambda: values[-1].command_result == MotorStatus.FAILED,
                 publish=lambda: pub.publish(command))
        assert 'rejected' in values[-1].fault
        wait_for(ros_node, lambda: translator.rev_s == 0)


def test_can_initialization_timeout_never_sends_motion(ros_node):
    packets = []
    ros_node.create_subscription(CANPacket, '/can_tx', packets.append, 100)
    values = statuses(ros_node)
    pub = ros_node.create_publisher(WheelCommand, '/Drive/FrontLeft', 1)
    with process('WheelMotor', *CAN_PARAMS):
        command = WheelCommand(velocity=2.0, steering_angle=0.0)
        wait_for(ros_node, lambda: values and 'initialization timed out' in values[-1].fault,
                 publish=lambda: pub.publish(command))
        assert not values[-1].ready
        assert not any(p.command == 0x89 for p in packets)
