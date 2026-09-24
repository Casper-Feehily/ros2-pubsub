# ROS 2 工作区

这是一个基于 ROS 2 Jazzy 的 C++ 工作区，功能包位于 `src/`。各包的构建、运行与验证方法见对应文档。

## 功能包

| 功能包 | 内容 | 文档 |
| --- | --- | --- |
| `rmcs_pubsub` | 发布和订阅 `rmcs_status` 消息 | [发布与订阅](src/rmcs_pubsub/README.md) |
| `sine_signal_generator` | 生成带噪声的正弦信号，并进行中值与低通滤波 | [信号发生与滤波](src/sine_signal_generator/README.md) |
| `motor_simulator` | 模拟二阶电机运动，并提供测试力矩源 | [电机模拟器](src/motor_simulator/README.md) |

## 开发入口

- Docker 构建及 CLion 调试步骤见 [发布与订阅文档](src/rmcs_pubsub/README.md)；调试镜像使用仓库中的 [Dockerfile.clion](Dockerfile.clion)。
- 各包的具体运行命令、参数和可视化步骤见上表对应文档。
