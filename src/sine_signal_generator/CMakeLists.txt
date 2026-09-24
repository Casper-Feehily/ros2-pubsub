cmake_minimum_required(VERSION 3.8)
project(sine_signal_generator)

find_package(ament_cmake REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rclcpp REQUIRED)

add_executable(sine_signal_generator src/sine_signal_generator.cpp)
ament_target_dependencies(sine_signal_generator geometry_msgs rclcpp)

add_executable(signal_filter src/signal_filter.cpp)
ament_target_dependencies(signal_filter geometry_msgs rclcpp)

install(TARGETS sine_signal_generator signal_filter DESTINATION lib/${PROJECT_NAME})

ament_package()
