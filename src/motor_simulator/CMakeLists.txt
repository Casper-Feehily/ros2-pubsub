
cmake_minimum_required(VERSION 3.8)
project(motor_simulator)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(motor_simulator src/motor_simulator.cpp)
ament_target_dependencies(motor_simulator rclcpp sensor_msgs std_msgs)

add_executable(torque_source src/torque_source.cpp)
ament_target_dependencies(torque_source rclcpp std_msgs)

install(TARGETS motor_simulator torque_source DESTINATION lib/${PROJECT_NAME})

ament_package()
