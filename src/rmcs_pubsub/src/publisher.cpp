#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

class Publisher : public rclcpp::Node
{
public:
  Publisher()
  : Node("publisher"), count_(0)
  {
    publisher_ = create_publisher<std_msgs::msg::String>("rmcs_status", 10);
    timer_ = create_wall_timer(1s, [this]() { publish_status(); });
  }

private:
  void publish_status()
  {
    std_msgs::msg::String message;
    message.data = "Hello World (" + std::to_string(count_++) + ")";
    publisher_->publish(message);
    RCLCPP_INFO(get_logger(), "Published: %s", message.data.c_str());
  }

  size_t count_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Publisher>());
  rclcpp::shutdown();
  return 0;
}
