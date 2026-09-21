import rclpy
from rclpy.node import Node
from std_msgs.msg import String


class Publisher(Node):
    def __init__(self):
        super().__init__('publisher') # 创建叫publisher的节点
        self.publisher = self.create_publisher(String, 'rmcs_status', 10) #创建一个叫rmcs_status的发布者
        self.count = 0
        self.create_timer(1.0, self.publish_status)

    def publish_status(self):
        message = String()
        message.data = f'Hello World: #{self.count}'
        self.publisher.publish(message)
        self.get_logger().info(f'Published: {message.data}') # 打印成功消息
        self.count += 1


def main(args=None):
    rclpy.init(args=args)
    node = Publisher()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()
