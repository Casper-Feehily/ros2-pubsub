
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

add_executable(pid_controller src/pid_controller.cpp)
target_include_directories(pid_controller PRIVATE include)
ament_target_dependencies(pid_controller rclcpp sensor_msgs std_msgs)

install(TARGETS motor_simulator torque_source pid_controller DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  enable_testing()
  add_executable(pid_test test/pid_test.cpp)
  target_include_directories(pid_test PRIVATE include)
  add_test(NAME pid_test COMMAND pid_test)

  add_executable(pid_integration_test test/pid_integration_test.cpp)
  ament_target_dependencies(pid_integration_test rclcpp sensor_msgs std_msgs)
  add_test(NAME pid_integration_test COMMAND pid_integration_test
    $<TARGET_FILE:motor_simulator> $<TARGET_FILE:pid_controller>)
  set_tests_properties(pid_integration_test PROPERTIES TIMEOUT 35)
endif()

ament_package()
