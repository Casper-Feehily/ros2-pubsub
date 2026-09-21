import rclpy
from rclpy.node import Node
from std_msgs.msg import String


class Subscriber(Node): # 创建叫subcriber的节点
    def __init__(self):
        super().__init__('subscriber')
        self.create_subscription(String, 'rmcs_status', self.on_message, 10) # 订阅发布者

    def on_message(self, message):
        self.get_logger().info(f'Received: {message.data}') # 接受成功打印信息


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
