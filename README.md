# SnakeTask

LiDAR 的独立官邸：具体 C++ 后端、Collector/协议/处理逻辑及其 QML 页面在同一仓库维护。来自 LiDAR_DataACQ master `5cc6d65`，分仓后的全功能入口保持原行为。

## 边界与接入

- QML 模块：`Lidar.SnakeTask`；页面：`SnakeTaskModule`。
- CMake 目标：`Lidar::SnakeTask`；静态插件：`LidarSnakeTaskplugin`，最终可执行文件显式 `Q_IMPORT_QML_PLUGIN(LidarSnakeTaskPlugin)`，并导入 TaskUI/UI/Charts 的静态插件。
- 页面以 attached 对象拥有唯一具体后端，后端不能在 QML 直接创建。设备通过只读 `family` 借出族引用；任务页面只接收族列表，不依赖设备官邸。
- 依赖由 `cmake/Dependencies.cmake` 固定到提交；可用标准 FetchContent 源码覆盖做联调。多个官邸并用时，先开启所有需要的 HandleCore/TaskCore 可选项，再配置共享库，避免已创建目标后补开模块。
- 不需要串口、抓包驱动。

## 构建与验证

C++20、CMake >= 3.28、Qt 6.11.2（Quick/Qml/GuiPrivate/ShaderTools）。Windows 使用 MSVC x64；Linux 使用 GCC，Qt 与原生依赖由环境提供。

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="<Qt前缀;需要的原生依赖前缀>"
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
cmake --build build --config Release --target LidarSnakeTask_qmllint
```

独立测试逐头编译、创建真实页面并销毁后端；不连接实物设备。硬件收发仍需对应设备实测。主程序负责跨官邸仿真、任务及取消链回归。

## 许可

MIT，Copyright (c) 2026 周涛（ZHOU TAO）。Qt、libserialport、libpcap/Npcap 等依赖各自授权，MIT 不替代这些许可；详见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。本仓库不包含第三方 SDK、运行库或驱动。

## 两套 QML 官邸（2026-10-27）

同一 QML 模块同时提供 `SnakeTaskModule`（全功能）和 `SnakeTaskSimOnlyModule`（数字样机）。每个页面以 Qt attached 机制拥有唯一的同类型 C++ 后端，没有第二套设备/任务实现。全功能主程序继续使用原入口。

simonly 页面接收 GMBDeviceBase gimbal 和 DAQDeviceBase card，保留本仓的唯一计划和独立协程，只提供计划/运行分区。启动固定保存为 true，目录 URL file:. 在启动边界按当前工作目录解释；不保存目录或开关副本，不显示设备选择。累积和停稳可编辑，虚拟云台速度来自设备配置，不额外覆盖任务速度。

[LiDAR_DataACQ-simonly](https://github.com/Lateef-ZHOU/LiDAR_DataACQ-simonly) 通过固定版本装配这些入口，负责 JSON、命令行、设备就绪后启动和系统信号取消。窗口隐藏仍保留历史，不触发实际绘制。测试创建两套真实页面并检查后端唯一性、销毁和精简分区；跨设备的三任务采样与保存由应用集成测试覆盖。
