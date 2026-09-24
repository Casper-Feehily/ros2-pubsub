# 电机仿真与双环 PID 控制

电机模型为 `J * theta_ddot + b * theta_dot = tau_control - tau_load`。`motor_simulator` 订阅 `motor/torque_cmd`（`std_msgs/msg/Float64`，N·m），并发布 `motor/state`（`sensor_msgs/msg/JointState`）：`position[0]` 是**连续累计**角度（rad），`velocity[0]` 是角速度（rad/s），`effort[0]` 是输入力矩（N·m）。默认积分频率为 1000 Hz。

`pid_controller` 有两种模式：

| 模式 | 输入 | 计算 | 输出 |
| --- | --- | --- | --- |
| `speed`（默认） | `motor/target_velocity`（rad/s）与速度反馈 | 速度 PID | `motor/torque_cmd`（N·m） |
| `position` | `motor/target_angle`（rad）与角度、速度反馈 | 角度 PID → 限幅目标速度 → 速度 PID | `motor/torque_cmd`（N·m） |

两种模式都会发布 `motor/controller/target_velocity`，即速度内环**实际使用的**目标速度。控制频率默认 600 Hz，给图片中建议的 500 Hz 留出调度余量；`control_rate` 不允许小于 500 Hz。计时使用单调时钟，PID 微分作用于测量值以避免目标阶跃产生微分冲击，输出有限幅和积分防饱和。没有收到有效反馈或目标、反馈超过 `feedback_timeout` 秒未更新时，控制器输出零力矩并清空 PID 状态。目标命令在收到下一条命令前保持有效；需要停止时请主动发送新的目标值。

## 构建

以下命令在 ROS 2 Jazzy 容器的 `/ws` 工作区执行：

```bash
source /opt/ros/jazzy/setup.bash
cd /ws
colcon build --packages-select motor_simulator --symlink-install
source /ws/install/setup.bash
```

每个新终端都要重新执行两条 `source` 命令。使用 Dev Container 时，配置已将宿主机的 `8765` 端口映射到容器。直接用 Docker 启动时需自行添加 `-p 8765:8765`。

## 速度闭环

分别在三个已加载 ROS 环境的终端运行：

```bash
ros2 run motor_simulator motor_simulator
```

```bash
ros2 run motor_simulator pid_controller
```

```bash
ros2 topic pub -r 10 /motor/target_velocity std_msgs/msg/Float64 '{data: 1.0}'
```

改变最后一条命令中的目标值，例如改为 `0.0`，观察制动和稳态表现。闭环运行时**不要同时启动** `torque_source`：它也会向 `motor/torque_cmd` 发布力矩，造成两个控制源竞争。`torque_source` 仍可单独用于开环仿真测试。

可用以下命令检查闭环与频率（`ros2 topic hz` 是观测值，受机器负载影响）：

```bash
ros2 topic hz /motor/torque_cmd
ros2 topic echo --once /motor/state
ros2 topic echo --once /motor/controller/target_velocity
```

电机参数和 PID 参数可以在启动时设置，例如：

```bash
ros2 run motor_simulator motor_simulator --ros-args -p inertia:=0.02 -p damping:=0.15 -p load_torque:=0.01
ros2 run motor_simulator pid_controller --ros-args -p speed_kp:=0.2 -p speed_ki:=0.5 -p max_torque:=0.2
```

可调参数包括 `speed_kp`、`speed_ki`、`speed_kd`、`position_kp`、`position_ki`、`position_kd`、`max_torque`、`max_speed`、`control_rate`、`feedback_timeout` 和 `mode`。PID 增益必须是有限的非负数，力矩和速度上限必须为正数。

## 双环角度控制与优弧

重启电机模拟器以将初始角度归零，启动控制器的位置模式，再发布**连续累计角度**目标：

```bash
ros2 run motor_simulator pid_controller --ros-args -p mode:=position
```

```bash
ros2 topic pub -r 10 /motor/target_angle std_msgs/msg/Float64 '{data: -4.71238898038469}'
```

从 0° 到几何位置 90°，直接发送 `+1.57079632679490` rad 会走逆时针短弧；发送 `-4.71238898038469` rad（−270°）会走顺时针优弧。控制器不将角度误差折叠到 `[-π, π]`，因此可按展开后的目标角度指定路径。运动中可观察 `motor/state.position[0]` 是否经过 −π 并最终接近 −3π/2。位置模式不要再向 `motor/target_velocity` 发布命令；速度内环的目标由角度外环生成。

## Foxglove 可视化

在同一个容器的另一终端启动 Bridge：

```bash
source /opt/ros/jazzy/setup.bash
source /ws/install/setup.bash
ros2 launch foxglove_bridge foxglove_bridge_launch.xml port:=8765
```

如已存在监听 `8765` 的 Bridge，直接复用即可。Foxglove Desktop 连接 `ws://localhost:8765`，在 Plot 面板添加：

| 曲线 | Y value | Timestamp |
| --- | --- | --- |
| 速度内环目标 | `/motor/controller/target_velocity.data` | Receive time |
| 实际速度 | `/motor/state.velocity[0]` | Header stamp |
| 控制力矩 | `/motor/torque_cmd.data` | Receive time |
| 累计角度（位置模式） | `/motor/state.position[0]` | Header stamp |

`motor/state` 带时间戳，较高发布频率下选 `Header stamp` 更容易看清曲线；两个 `Float64` 话题没有时间戳，选 `Receive time`。速度与力矩单位不同，建议分别放在两个 Plot 面板中。截图应包含目标速度与实际速度、力矩限幅；进阶部分再附累计角度越过 −π 的曲线。

## 测试

```bash
colcon test --packages-select motor_simulator --ctest-args -R 'pid_(test|integration_test)'
colcon test-result --verbose
```

`pid_test` 用确定性的二阶电机模型检查速度阶跃跟踪、力矩限幅和 −270° 的优弧角度跟踪。`pid_integration_test` 用 C++ 启动真实 ROS 2 节点，检查话题通信、跟踪结果与实际发布频率。Foxglove 曲线仍需在桌面端查看。
