#include <chrono>
#include <cmath>
#include <memory>
#include <stdexcept>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float64.hpp"

class TorqueSource : public rclcpp::Node
{
public:
  TorqueSource()
  : Node("torque_source")
  {
    amplitude_ = declare_parameter<double>("amplitude", 0.2);
    const auto period = declare_parameter<double>("period", 4.0);
    const auto publish_rate = declare_parameter<double>("publish_rate", 100.0);
    if (period <= 0.0 || publish_rate <= 0.0) {
      throw std::invalid_argument("period and publish_rate must be positive");
    }

    angular_frequency_ = 2.0 * 3.14159265358979323846 / period;
    publisher_ = create_publisher<std_msgs::msg::Float64>("motor/torque_cmd", 10);
    start_time_ = get_clock()->now();
    const auto timer_period = std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::duration<double>(1.0 / publish_rate));
    timer_ = create_wall_timer(timer_period, [this]() { publish_torque(); });
  }

private:
  void publish_torque()
  {
    std_msgs::msg::Float64 command;
    command.data = amplitude_ * std::sin(angular_frequency_ * (get_clock()->now() - start_time_).seconds());
    publisher_->publish(command);
  }

  double amplitude_;
  double angular_frequency_;
  rclcpp::Time start_time_;
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TorqueSource>());
  rclcpp::shutdown();
  return 0;
}
