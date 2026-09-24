#include <chrono>
#include <memory>
#include <stdexcept>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "std_msgs/msg/float64.hpp"

class MotorSimulator : public rclcpp::Node
{
public:
  MotorSimulator()
  : Node("motor_simulator"), last_step_(std::chrono::steady_clock::now())
  {
    inertia_ = declare_parameter<double>("inertia", 0.01);
    damping_ = declare_parameter<double>("damping", 0.1);
    load_torque_ = declare_parameter<double>("load_torque", 0.0);
    const auto update_rate = declare_parameter<double>("update_rate", 1000.0);
    if (inertia_ <= 0.0 || damping_ < 0.0 || update_rate <= 0.0) {
      throw std::invalid_argument("inertia and update_rate must be positive; damping must be non-negative");
    }

    torque_subscriber_ = create_subscription<std_msgs::msg::Float64>(
      "motor/torque_cmd", 10,
      [this](const std_msgs::msg::Float64::SharedPtr message) { control_torque_ = message->data; });
    state_publisher_ = create_publisher<sensor_msgs::msg::JointState>("motor/state", 10);

    const auto period = std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::duration<double>(1.0 / update_rate));
    timer_ = create_wall_timer(period, [this]() { step(); });
  }

private:
  void step()
  {
    const auto now = std::chrono::steady_clock::now();
    const double dt = std::chrono::duration<double>(now - last_step_).count();
    last_step_ = now;

    // J * theta_ddot + b * theta_dot = tau_control - tau_load.
    const double acceleration = (control_torque_ - load_torque_ - damping_ * angular_velocity_) / inertia_;
    angular_velocity_ += acceleration * dt;
    rotor_angle_ += angular_velocity_ * dt;  // Semi-implicit Euler integration.

    sensor_msgs::msg::JointState state;
    state.header.stamp = get_clock()->now();
    state.name = {"motor_rotor"};
    state.position = {rotor_angle_};
    state.velocity = {angular_velocity_};
    state.effort = {control_torque_};
    state_publisher_->publish(state);
  }

  double inertia_;
  double damping_;
  double load_torque_;
  double control_torque_{0.0};
  double rotor_angle_{0.0};
  double angular_velocity_{0.0};
  std::chrono::steady_clock::time_point last_step_;
  rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr torque_subscriber_;
  rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr state_publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MotorSimulator>());
  rclcpp::shutdown();
  return 0;
}
