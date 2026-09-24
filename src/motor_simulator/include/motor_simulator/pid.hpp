#pragma once

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace motor_simulator
{

class Pid
{
public:
  struct Config
  {
    double kp;
    double ki;
    double kd;
    double output_limit;
  };

  explicit Pid(Config config) : config_(config)
  {
    if (!std::isfinite(config_.kp) || !std::isfinite(config_.ki) ||
        !std::isfinite(config_.kd) || !std::isfinite(config_.output_limit) ||
        config_.kp < 0.0 || config_.ki < 0.0 || config_.kd < 0.0 ||
        config_.output_limit <= 0.0) {
      throw std::invalid_argument("PID gains must be finite and non-negative; output_limit must be positive");
    }
  }

  double update(double target, double measurement, double dt)
  {
    if (!std::isfinite(target) || !std::isfinite(measurement) ||
        !std::isfinite(dt) || dt <= 0.0) {
      throw std::invalid_argument("PID inputs must be finite and dt must be positive");
    }

    const double error = target - measurement;
    const double derivative = config_.kd > 0.0 && has_previous_measurement_ ?
      -(measurement - previous_measurement_) / dt : 0.0;
    const double proposed_integral = config_.ki > 0.0 ? integral_ + error * dt : 0.0;
    if (!std::isfinite(error) || !std::isfinite(derivative) ||
        !std::isfinite(proposed_integral)) {
      throw std::invalid_argument("PID calculation overflowed");
    }
    double output = config_.kp * error + config_.ki * proposed_integral +
      config_.kd * derivative;
    if (!std::isfinite(output)) {
      throw std::invalid_argument("PID output is not finite");
    }

    // Do not integrate further into saturation; allow integration back out.
    if (!((output > config_.output_limit && error > 0.0) ||
          (output < -config_.output_limit && error < 0.0))) {
      integral_ = proposed_integral;
    }
    output = config_.kp * error + config_.ki * integral_ + config_.kd * derivative;
    if (!std::isfinite(output)) {
      throw std::invalid_argument("PID output is not finite");
    }

    previous_measurement_ = measurement;
    has_previous_measurement_ = true;
    return std::clamp(output, -config_.output_limit, config_.output_limit);
  }

  void reset()
  {
    integral_ = 0.0;
    previous_measurement_ = 0.0;
    has_previous_measurement_ = false;
  }

private:
  Config config_;
  double integral_{0.0};
  double previous_measurement_{0.0};
  bool has_previous_measurement_{false};
};

}  // namespace motor_simulator
