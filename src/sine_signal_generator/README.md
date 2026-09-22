# sine_signal_generator

发布 `std_msgs/msg/Float64` 类型的 `noisy_sine` topic：20 Hz 正弦信号加上标准差为幅值 1% 的高斯白噪声。

默认发布频率为 1000 Hz。构建后运行：

```bash
ros2 run sine_signal_generator sine_signal_generator
```

示例：将幅值设为 3、发布率设为 800 Hz：

```bash
ros2 run sine_signal_generator sine_signal_generator --ros-args -p amplitude:=3.0 -p publish_rate:=800.0
```
