import rclpy
from rclpy.node import Node
from std_msgs.msg import String


class Subscriber(Node):
    def __init__(self):
        super().__init__('subscriber')
        self.create_subscription(String, 'rmcs_status', self.on_message, 10)

    def on_message(self, message):
        self.get_logger().info(f'Received: {message.data}')


def main(args=None):
    rclpy.init(args=args)
    node = Subscriber()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()
