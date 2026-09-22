cmake_minimum_required(VERSION 3.8)
project(sine_signal_generator)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(sine_signal_generator src/sine_signal_generator.cpp)
ament_target_dependencies(sine_signal_generator rclcpp std_msgs)

install(TARGETS sine_signal_generator DESTINATION lib/${PROJECT_NAME})

ament_package()
