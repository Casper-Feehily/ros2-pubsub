# ROS 2 发布者与订阅者

`rmcs_pubsub` 包提供两个独立节点：

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

分别打开两个终端；每个终端先进入同一个容器并执行上面的两条 `source` 命令，然后运行：

```bash
ros2 run rmcs_pubsub publisher
ros2 run rmcs_pubsub subscriber
```

订阅者会持续显示 `Received: RMCS ROS 2 is ready: #N`。
