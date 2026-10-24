#!/usr/bin/env python3

# Basic node for acting as a CLI controller for rover functinoality import rclpy
import rclpy
from rclpy.node import Node

from pynput import keyboard

from std_msgs.msg import String
from geometry_msgs.msg import Twist


class Controller(Node):

    def __init__(self):
        super().__init__('controller')
        self.direction_publisher_ = self.create_publisher(Twist, 'direction', 10)
        self.vector_ = Twist() # Desired vector

        # Setup default vector
        self.vector_.linear.x = self.vector_.linear.y = self.vector_.linear.z = 0.0 
        self.vector_.angular.x = self.vector_.angular.y = self.vector_.angular.z = 0.0 
        self.direction_publisher_.publish(self.vector_)

    def on_press(self, key):
        if key == keyboard.KeyCode.from_char("w"):
            self.vector_.linear.y = 1.0
        if key == keyboard.KeyCode.from_char("a"):
            self.vector_.linear.x = -1.0
        if key == keyboard.KeyCode.from_char("s"):
            self.vector_.linear.y = -1.0
        if key == keyboard.KeyCode.from_char("d"):
            self.vector_.linear.y = 1.0

        print("Current vec " + str(self.vector_))

        self.direction_publisher_.publish(self.vector_)
        pass

    def on_release(self, key):
        # I am lazy, prob reset only for lifted key
        self.vector_.linear.x = self.vector_.linear.y = self.vector_.linear.z = 0.0 
        self.vector_.angular.x = self.vector_.angular.y = self.vector_.angular.z = 0.0 
        pass

    


def main(args=None):
    rclpy.init(args=args)

    controller = Controller()

    listener = keyboard.Listener(
        on_press=controller.on_press,
        on_release=controller.on_release)

    listener.start()

    rclpy.spin(controller)

    # Destroy the node explicitly
    # (optional - otherwise it will be done automatically
    # when the garbage collector destroys the node object)
    controller.destroy_node()
    rclpy.shutdown()




if __name__ == '__main__':
    main()