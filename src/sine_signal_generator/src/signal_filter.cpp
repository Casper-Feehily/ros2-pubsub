#include <algorithm>
#include <cstdint>
#include <deque>
#include <memory>
#include <stdexcept>
#include <vector>

#include "geometry_msgs/msg/vector3_stamped.hpp"
#include "rclcpp/rclcpp.hpp"

class SignalFilter : public rclcpp::Node
{
public:
  SignalFilter()
  : Node("signal_filter")
  {
    const auto median_window = declare_parameter<int64_t>("median_window", 5);
    alpha_ = declare_parameter<double>("low_pass_alpha", 0.1);
    if (median_window <= 0 || median_window % 2 == 0 || alpha_ <= 0.0 || alpha_ > 1.0) {
      throw std::invalid_argument("median_window must be positive and odd; low_pass_alpha must be in (0, 1]");
    }

    median_window_ = static_cast<size_t>(median_window);
    publisher_ = create_publisher<geometry_msgs::msg::Vector3Stamped>("filtered_sine", 10);
    subscription_ = create_subscription<geometry_msgs::msg::Vector3Stamped>(
      "noisy_sine", 10, [this](geometry_msgs::msg::Vector3Stamped::SharedPtr message) {
        filter_and_publish(*message);
      });
  }

private:
  void filter_and_publish(const geometry_msgs::msg::Vector3Stamped & input)
  {
    samples_.push_back(input.vector.x);
    if (samples_.size() > median_window_) {
      samples_.pop_front();
    }

    std::vector<double> sorted(samples_.begin(), samples_.end());
    std::sort(sorted.begin(), sorted.end());
    const double median = sorted[sorted.size() / 2];
    filtered_value_ = has_filtered_value_ ? alpha_ * median + (1.0 - alpha_) * filtered_value_ : median;
    has_filtered_value_ = true;

    geometry_msgs::msg::Vector3Stamped output;
    output.header = input.header;
    output.vector.x = filtered_value_;
    publisher_->publish(output);
  }

  size_t median_window_;
  double alpha_;
  double filtered_value_{0.0};
  bool has_filtered_value_{false};
  std::deque<double> samples_;
  rclcpp::Subscription<geometry_msgs::msg::Vector3Stamped>::SharedPtr subscription_;
  rclcpp::Publisher<geometry_msgs::msg::Vector3Stamped>::SharedPtr publisher_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SignalFilter>());
  rclcpp::shutdown();
  return 0;
}
