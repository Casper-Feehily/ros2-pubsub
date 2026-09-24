# ROS 2 发布者与订阅者

`rmcs_pubsub` 是一个 `ament_cmake` C++ ROS 2 包，提供两个独立节点：

- `publisher` 每秒向 `rmcs_status` topic 发布一条 `std_msgs/msg/String` 消息。
- `subscriber` 订阅该 topic，并用 ROS 2 Logger 输出接收到的内容。

## 在 Docker 中构建

在工作区根目录执行：

```bash
docker run --rm -it --name rmcs-ros2 -v "$PWD":/ws -w /ws qzhhhi/rmcs-develop:latest bash
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

## 在 CLion 中运行和调试

先在工作区根目录构建带 GDB 的本地镜像：

```bash
docker build -f Dockerfile.clion -t rmcs-develop-clion:gdb .
```

在 CLion 的 **设置 → 构建、执行、部署 → 工具链** 中选择 Docker 工具链，镜像设为 `rmcs-develop-clion:gdb`，并在 **Add environment → From file** 中填入 `/opt/ros/jazzy/setup.bash`。在 **CMake** 中让 Debug 配置使用该 Docker 工具链。打开要开发的包的 `CMakeLists.txt` 作为 CMake 项目，建立 **CMake Application** 运行配置，目标选择节点的 CMake target；调试配置文件选 **GDB**。

不要使用编辑器自动生成的 **C/C++ File** 配置。它只会对单个 `.cpp` 运行 `c++`，不会加载 `ament_cmake` 中声明的 ROS 依赖，因此找不到 `geometry_msgs` 等头文件。其他 ROS 包也按各自的 `CMakeLists.txt` 和节点 target 建立运行配置。
