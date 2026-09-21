# ROS 2 发布者与订阅者

`rmcs_pubsub` 是一个 `ament_cmake` C++ ROS 2 包，提供两个独立节点：

- `publisher` 每秒向 `rmcs_status` topic 发布一条 `std_msgs/msg/String` 消息。
- `subscriber` 订阅该 topic，并用 ROS 2 Logger 输出接收到的内容。

## 在 Docker 中构建

在 `ros2_ws` 目录执行：

```bash
docker run --rm -it -v "$PWD":/ws -w /ws qzhhhi/rmcs-develop:latest bash
source /opt/ros/jazzy/setup.bash
colcon build --symlink-install
source install/setup.bash
```

在第一个终端运行发布者：

```bash
ros2 run rmcs_pubsub publisher
```

再打开一个终端，进入同一个运行中的容器：

```bash
docker exec -it rmcs-ros2 bash
source /opt/ros/jazzy/setup.bash
source /ws/install/setup.bash
ros2 run rmcs_pubsub subscriber
```

订阅者会持续显示 `Received: Hello World (N)`。
