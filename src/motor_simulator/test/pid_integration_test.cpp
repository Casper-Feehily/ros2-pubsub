#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <csignal>
#include <sys/wait.h>
#include <unistd.h>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "std_msgs/msg/float64.hpp"

using Clock = std::chrono::steady_clock;

class ChildProcess
{
public:
  ChildProcess(const char * executable, const std::string & ros_namespace,
    const std::string & mode = "")
  {
    pid_ = fork();
    if (pid_ < 0) {
      throw std::runtime_error("fork failed");
    }
    if (pid_ == 0) {
      const std::string namespace_arg = "__ns:=/" + ros_namespace;
      if (mode.empty()) {
        execl(executable, executable, "--ros-args", "-r", namespace_arg.c_str(),
          static_cast<char *>(nullptr));
      } else {
        const std::string mode_arg = "mode:=" + mode;
        execl(executable, executable, "--ros-args", "-r", namespace_arg.c_str(),
          "-p", mode_arg.c_str(), static_cast<char *>(nullptr));
      }
      _exit(127);
    }
  }

  ChildProcess(const ChildProcess &) = delete;
  ChildProcess & operator=(const ChildProcess &) = delete;

  ~ChildProcess()
  {
    if (pid_ > 0) {
      kill(pid_, SIGTERM);
      for (int i = 0; i < 30; ++i) {
        if (waitpid(pid_, nullptr, WNOHANG) != 0) {
          return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
      kill(pid_, SIGKILL);
      waitpid(pid_, nullptr, 0);
    }
  }

private:
  pid_t pid_{-1};
};

void require(bool condition, const std::string & message)
{
  if (!condition) {
    throw std::runtime_error(message);
  }
}

struct StateSample
{
  Clock::time_point time;
  double angle;
  double speed;
};

struct TorqueSample
{
  Clock::time_point time;
  double torque;
};

void run_mode(const char * simulator_path, const char * controller_path,
  const std::string & mode)
{
  const std::string ros_namespace = "pid_test_" + std::to_string(getpid()) + "_" + mode;
  ChildProcess simulator(simulator_path, ros_namespace);
  ChildProcess controller(controller_path, ros_namespace, mode);

  auto node = std::make_shared<rclcpp::Node>("probe", "/" + ros_namespace);
  std::vector<StateSample> states;
  std::vector<TorqueSample> torques;
  auto state_subscription = node->create_subscription<sensor_msgs::msg::JointState>(
    "motor/state", 2000,
    [&states](const sensor_msgs::msg::JointState::SharedPtr message) {
      if (!message->position.empty() && !message->velocity.empty()) {
        states.push_back({Clock::now(), message->position[0], message->velocity[0]});
      }
    });
  auto torque_subscription = node->create_subscription<std_msgs::msg::Float64>(
    "motor/torque_cmd", 2000,
    [&torques](const std_msgs::msg::Float64::SharedPtr message) {
      torques.push_back({Clock::now(), message->data});
    });
  const std::string target_topic = mode == "speed" ?
    "motor/target_velocity" : "motor/target_angle";
  auto target_publisher = node->create_publisher<std_msgs::msg::Float64>(target_topic, 10);
  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);
  std::thread spin_thread([&executor]() { executor.spin(); });

  const double target = mode == "speed" ? 1.0 : -1.5 * std::acos(-1.0);
  const auto duration = mode == "speed" ? std::chrono::seconds(4) : std::chrono::seconds(10);
  const auto start = Clock::now();
  while (Clock::now() - start < duration) {
    std_msgs::msg::Float64 command;
    command.data = target;
    target_publisher->publish(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  executor.cancel();
  spin_thread.join();

  require(!states.empty() && !torques.empty(), "missing ROS state or torque messages");
  double steady_sum = 0.0;
  int steady_count = 0;
  double minimum_angle = 0.0;
  for (const auto & sample : states) {
    minimum_angle = std::min(minimum_angle, sample.angle);
    if (sample.time >= start + duration - std::chrono::milliseconds(500)) {
      steady_sum += mode == "speed" ? sample.speed : sample.angle;
      ++steady_count;
    }
  }
  require(steady_count > 0, "no recent state samples");
  const double steady = steady_sum / steady_count;
  double max_torque = 0.0;
  int rate_count = 0;
  for (const auto & sample : torques) {
    max_torque = std::max(max_torque, std::abs(sample.torque));
    if (sample.time >= start + std::chrono::seconds(1) &&
        sample.time < start + std::chrono::seconds(3)) {
      ++rate_count;
    }
  }
  const double observed_rate = rate_count / 2.0;
  std::cout << "mode=" << mode << " steady=" << steady << " max_torque=" << max_torque
            << " torque_messages_per_second=" << observed_rate << std::endl;
  require(max_torque <= 0.20001, "torque exceeded limit");
  require(observed_rate >= 500.0, "control output rate below 500 Hz");
  if (mode == "speed") {
    require(std::abs(steady - target) < 0.05, "speed did not track target");
  } else {
    require(minimum_angle < -std::acos(-1.0), "did not traverse long arc");
    require(std::abs(steady - target) < 0.08, "position did not reach target");
  }
}

int main(int argc, char * argv[])
{
  if (argc != 3) {
    std::cerr << "usage: pid_integration_test MOTOR_SIMULATOR PID_CONTROLLER\n";
    return 2;
  }
  rclcpp::init(argc, argv);
  try {
    run_mode(argv[1], argv[2], "speed");
    run_mode(argv[1], argv[2], "position");
  } catch (const std::exception & error) {
    std::cerr << error.what() << std::endl;
    rclcpp::shutdown();
    return 1;
  }
  rclcpp::shutdown();
  return 0;
}
