#include <chrono>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>

#include "motor_simulator/pid.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "std_msgs/msg/float64.hpp"

class PidController : public rclcpp::Node
{
public:
  PidController()
  : Node("pid_controller"), last_tick_(std::chrono::steady_clock::now())
  {
    mode_ = declare_parameter<std::string>("mode", "speed");
    const double control_rate = declare_parameter<double>("control_rate", 600.0);
    feedback_timeout_ = declare_parameter<double>("feedback_timeout", 0.1);
    const double max_torque = declare_parameter<double>("max_torque", 0.2);
    const double max_speed = declare_parameter<double>("max_speed", 3.0);
    const motor_simulator::Pid::Config speed_config{
      declare_parameter<double>("speed_kp", 0.1),
      declare_parameter<double>("speed_ki", 0.5),
      declare_parameter<double>("speed_kd", 0.0),
      max_torque};
    const motor_simulator::Pid::Config position_config{
      declare_parameter<double>("position_kp", 2.0),
      declare_parameter<double>("position_ki", 0.05),
      declare_parameter<double>("position_kd", 0.1),
      max_speed};

    if ((mode_ != "speed" && mode_ != "position") ||
        !std::isfinite(control_rate) || control_rate < 500.0 ||
        !std::isfinite(feedback_timeout_) || feedback_timeout_ <= 0.0) {
      throw std::invalid_argument("mode must be speed or position; control_rate must be >= 500 Hz; feedback_timeout must be positive");
    }
    speed_pid_ = std::make_unique<motor_simulator::Pid>(speed_config);
    position_pid_ = std::make_unique<motor_simulator::Pid>(position_config);

    state_subscriber_ = create_subscription<sensor_msgs::msg::JointState>(
      "motor/state", 10,
      [this](const sensor_msgs::msg::JointState::SharedPtr state) {
        if (state->position.empty() || state->velocity.empty() ||
            !std::isfinite(state->position[0]) || !std::isfinite(state->velocity[0])) {
          return;
        }
        angle_ = state->position[0];
        speed_ = state->velocity[0];
        has_feedback_ = true;
        last_feedback_ = std::chrono::steady_clock::now();
      });
    speed_target_subscriber_ = create_subscription<std_msgs::msg::Float64>(
      "motor/target_velocity", 10,
      [this](const std_msgs::msg::Float64::SharedPtr target) {
        if (std::isfinite(target->data)) {
          speed_target_ = target->data;
          has_speed_target_ = true;
        }
      });
    angle_target_subscriber_ = create_subscription<std_msgs::msg::Float64>(
      "motor/target_angle", 10,
      [this](const std_msgs::msg::Float64::SharedPtr target) {
        if (std::isfinite(target->data)) {
          angle_target_ = target->data;
          has_angle_target_ = true;
        }
      });
    torque_publisher_ = create_publisher<std_msgs::msg::Float64>("motor/torque_cmd", 10);
    inner_target_publisher_ = create_publisher<std_msgs::msg::Float64>(
      "motor/controller/target_velocity", 10);

    const auto period = std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::duration<double>(1.0 / control_rate));
    timer_ = create_wall_timer(period, [this]() { control_step(); });
  }

private:
  void publish(double torque, double inner_target)
  {
    std_msgs::msg::Float64 torque_message;
    torque_message.data = torque;
    torque_publisher_->publish(torque_message);
    std_msgs::msg::Float64 target_message;
    target_message.data = inner_target;
    inner_target_publisher_->publish(target_message);
  }

  void control_step()
  {
    const auto now = std::chrono::steady_clock::now();
    const double dt = std::chrono::duration<double>(now - last_tick_).count();
    last_tick_ = now;
    const bool has_target = mode_ == "speed" ? has_speed_target_ : has_angle_target_;
    if (!has_feedback_ || !has_target || dt <= 0.0 || dt > feedback_timeout_ ||
        std::chrono::duration<double>(now - last_feedback_).count() > feedback_timeout_) {
      speed_pid_->reset();
      position_pid_->reset();
      publish(0.0, 0.0);
      return;
    }

    // The outer position loop uses the simulator's unwrapped angle. A target
    // such as -3*pi/2 from zero therefore follows the long clockwise arc.
    try {
      const double inner_target = mode_ == "position" ?
        position_pid_->update(angle_target_, angle_, dt) : speed_target_;
      publish(speed_pid_->update(inner_target, speed_, dt), inner_target);
    } catch (const std::invalid_argument & error) {
      speed_pid_->reset();
      position_pid_->reset();
      RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 1000,
        "PID calculation rejected: %s", error.what());
      publish(0.0, 0.0);
    }
  }

  std::string mode_;
  double feedback_timeout_;
  double angle_{0.0};
  double speed_{0.0};
  double speed_target_{0.0};
  double angle_target_{0.0};
  bool has_feedback_{false};
  bool has_speed_target_{false};
  bool has_angle_target_{false};
  std::chrono::steady_clock::time_point last_tick_;
  std::chrono::steady_clock::time_point last_feedback_;
  std::unique_ptr<motor_simulator::Pid> speed_pid_;
  std::unique_ptr<motor_simulator::Pid> position_pid_;
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr state_subscriber_;
  rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr speed_target_subscriber_;
  rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr angle_target_subscriber_;
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr torque_publisher_;
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr inner_target_publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<PidController>());
  rclcpp::shutdown();
  return 0;
}
