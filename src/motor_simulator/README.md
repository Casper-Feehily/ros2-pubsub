# 二阶电机模拟器

模拟模型为 `J * theta_ddot + b * theta_dot = tau_control - tau_load`。`motor/torque_cmd` 接收 `std_msgs/msg/Float64`（N·m）；`motor/state` 发布 `sensor_msgs/msg/JointState`：`position[0]` 是角度（rad），`velocity[0]` 是角速度（rad/s）。


```bash
colcon build --packages-select motor_simulator --symlink-install
source install/setup.bash
```

在三个终端分别运行：

```bash
source /opt/ros/jazzy/setup.bash
source /ws/install/setup.bash
ros2 run motor_simulator motor_simulator

source /opt/ros/jazzy/setup.bash
source /ws/install/setup.bash
ros2 run motor_simulator torque_source

source /opt/ros/jazzy/setup.bash
ros2 run foxglove_bridge foxglove_bridge
```

`torque_source` 默认发布周期为 4 s、幅值为 0.2 N·m 的正弦力矩。物理参数可在启动时调节，例如：

```bash
ros2 run motor_simulator motor_simulator --ros-args -p inertia:=0.02 -p damping:=0.15 -p load_torque:=0.01
```
