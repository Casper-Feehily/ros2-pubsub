#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>

#include "motor_simulator/pid.hpp"

using motor_simulator::Pid;

void require(bool condition, const std::string & message)
{
  if (!condition) {
    throw std::runtime_error(message);
  }
}

int main()
{
  bool rejected_invalid_config = false;
  try {
    Pid invalid({-1.0, 0.0, 0.0, 1.0});
  } catch (const std::invalid_argument &) {
    rejected_invalid_config = true;
  }
  require(rejected_invalid_config, "negative PID gain was accepted");

  bool rejected_overflow = false;
  try {
    Pid finite({1.0, 0.0, 0.0, 1.0});
    finite.update(std::numeric_limits<double>::max(),
      -std::numeric_limits<double>::max(), 0.002);
  } catch (const std::invalid_argument &) {
    rejected_overflow = true;
  }
  require(rejected_overflow, "overflowing PID error was accepted");

  Pid saturated({0.0, 1.0, 0.0, 1.0});
  for (int i = 0; i < 100; ++i) {
    require(saturated.update(10.0, 0.0, 0.1) == 1.0, "output was not saturated");
  }
  // The integrator must recover immediately when the error reverses.
  require(saturated.update(0.0, 1.0, 0.1) < 1.0, "integrator did not recover");

  Pid speed_pid({0.1, 0.5, 0.0, 0.2});
  double speed = 0.0;
  double torque = 0.0;
  constexpr double dt = 0.001;
  for (int tick = 0; tick < 3000; ++tick) {
    if (tick % 2 == 0) {
      torque = speed_pid.update(1.0, speed, 2.0 * dt);
      require(std::abs(torque) <= 0.2, "speed-loop torque exceeded limit");
    }
    speed += ((torque - 0.1 * speed) / 0.01) * dt;
  }
  require(std::abs(speed - 1.0) < 0.05, "speed loop did not track target");

  Pid position_pid({2.0, 0.05, 0.1, 3.0});
  speed_pid.reset();
  speed = 0.0;
  torque = 0.0;
  double angle = 0.0;
  constexpr double long_arc_target = -4.71238898038469;
  for (int tick = 0; tick < 10000; ++tick) {
    if (tick % 2 == 0) {
      const double target_speed = position_pid.update(long_arc_target, angle, 2.0 * dt);
      torque = speed_pid.update(target_speed, speed, 2.0 * dt);
      require(std::abs(target_speed) <= 3.0, "position-loop speed exceeded limit");
      require(std::abs(torque) <= 0.2, "position-loop torque exceeded limit");
    }
    speed += ((torque - 0.1 * speed) / 0.01) * dt;
    angle += speed * dt;
  }
  require(angle < -3.141592653589793, "position did not traverse long arc");
  require(std::abs(angle - long_arc_target) < 0.05, "position did not reach target");
}
