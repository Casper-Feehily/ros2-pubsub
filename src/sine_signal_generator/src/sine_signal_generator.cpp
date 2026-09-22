#include <chrono>
#include <cmath>
#include <memory>
#include <random>
#include <stdexcept>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float64.hpp"

class SineSignalGenerator : public rclcpp::Node
{
public:
  SineSignalGenerator()
  : Node("sine_signal_generator"), rng_(std::random_device{}())
  {
    amplitude_ = declare_parameter<double>("amplitude", 1.0);
    const auto signal_frequency = declare_parameter<double>("signal_frequency", 20.0);
    const auto publish_rate = declare_parameter<double>("publish_rate", 1000.0);

    if (amplitude_ <= 0.0 || signal_frequency <= 0.0 || publish_rate <= 0.0) {
      throw std::invalid_argument("amplitude, signal_frequency, and publish_rate must be positive");
    }

    angular_frequency_ = 2.0 * 3.14159265358979323846 * signal_frequency;
    noise_ = std::normal_distribution<double>(0.0, amplitude_ * 0.01);
    publisher_ = create_publisher<std_msgs::msg::Float64>("noisy_sine", 10);
    start_time_ = get_clock()->now();
    timer_ = create_wall_timer(
      std::chrono::duration<double>(1.0 / publish_rate),
      [this]() { publish_signal(); });
  }

private:
  void publish_signal()
  {
    const double t = (get_clock()->now() - start_time_).seconds();
    std_msgs::msg::Float64 message;
    message.data = amplitude_ * std::sin(angular_frequency_ * t) + noise_(rng_);
    publisher_->publish(message);
  }

  double amplitude_;
  double angular_frequency_;
  rclcpp::Time start_time_;
  std::mt19937 rng_;
  std::normal_distribution<double> noise_;
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SineSignalGenerator>());
  rclcpp::shutdown();
  return 0;
}
