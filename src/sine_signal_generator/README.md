> 注：本README.md绝大部分由ai生成
# sine_signal_generator

一个 ROS 2 数据发生与滤波包：以高发布频率产生带高斯白噪声的 20 Hz 正弦信号，并提供中值滤波和一阶低通滤波结果，供 Foxglove 可视化。

## 数据流

```text
sine_signal_generator
    /noisy_sine (Vector3Stamped, vector.x)
                |
                v
signal_filter: median filter -> low-pass filter
    /filtered_sine (Vector3Stamped, vector.x)
```

两个 topic 均为 `geometry_msgs/msg/Vector3Stamped`。数值放在 `vector.x`；每条消息都带有 `header.stamp`，因此适合高频绘图。

## 构建

在工作区根目录执行：

```bash
source /opt/ros/jazzy/setup.bash
colcon build --packages-select sine_signal_generator --symlink-install
source install/setup.bash
```

每次新开终端后，都需要重新执行两条 `source` 命令。

## 运行节点

### 1. 启动带噪声正弦信号发生器

```bash
ros2 run sine_signal_generator sine_signal_generator
```

默认值：幅值 `1.0`、信号频率 `20 Hz`、发布频率 `1000 Hz`、噪声标准差为幅值的 `1%`。

例如，将幅值设为 `3`、发布频率设为 `800 Hz`：

```bash
ros2 run sine_signal_generator sine_signal_generator --ros-args \
  -p amplitude:=3.0 \
  -p publish_rate:=800.0
```

### 2. 启动滤波节点

另开一个已 source 环境的终端：

```bash
ros2 run sine_signal_generator signal_filter
```

处理顺序是：**中值滤波 → 一阶低通滤波**。默认中值窗口为 `5`，低通系数为 `0.1`。

```bash
ros2 run sine_signal_generator signal_filter --ros-args \
  -p median_window:=7 \
  -p low_pass_alpha:=0.2
```

`median_window` 必须为正奇数；`low_pass_alpha` 必须在 `(0, 1]`。系数越小，曲线越平滑，但相位滞后也越明显。

## 验证 ROS 2 topic

```bash
ros2 topic hz /noisy_sine
ros2 topic echo --once /filtered_sine
```

发生器本身不会持续打印日志；用上述命令检查发布频率和滤波输出即可。

## Foxglove Bridge 与绘图

### 1. 启动 Bridge

在与两个节点相同的 ROS 2 环境中，另开终端执行：

```bash
source /opt/ros/jazzy/setup.bash
source /ws/install/setup.bash
ros2 launch foxglove_bridge foxglove_bridge_launch.xml port:=8765
```

若提示找不到 `foxglove_bridge`，在 ROS 2 Jazzy 容器内安装：

```bash
apt update
apt install -y ros-jazzy-foxglove-bridge
```

如果端口 `8765` 已被占用，不要再启动一个 Bridge。先检查已有监听者：

```bash
ss -ltnp | grep ':8765'
```

### 2. 连接 Foxglove

确认 Docker 或 Dev Container 已将容器端口 `8765` 转发到本机后，在 Foxglove Desktop 选择 **Open connection**，使用：

```text
ws://localhost:8765
```

### 3. 配置 Plot 面板

添加一个 Plot 面板，并添加两条曲线：

| 曲线 | Y value | Timestamp |
| --- | --- | --- |
| 原始带噪信号 | `/noisy_sine.vector.x` | `Header stamp` |
| 滤波后信号 | `/filtered_sine.vector.x` | `Header stamp` |

推荐将显示时间窗口设置为 `0.5` 到 `1` 秒。发布频率较高时必须选择 `Header stamp`，否则可能显示为密集的竖线。
