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

创建 simonly 页面时还必须给出 `azimuths`（世界方位角列表）、`elevation`（世界高度角）、`accum` 和 `settle`（秒）。角度属性直接 alias 到唯一计划页，时间属性由官邸持有；预览、输入框和运行共用这些值，不缓存第二份启动参数。缺少任一项时 QML 拒绝创建；业务有效性仍由已有校验负责。

官邸公开无参 `start(): bool` 和 `stop(): void`。自动启动与开始按钮调用同一个 `start()`，官邸从当前页面读取参数并交给既有 C++ 启动入口形成快照。运行中的编辑仍只影响下次启动；全功能页面、C++ 启动接口和独立协程保持不变。

[LiDAR_DataACQ-simonly](https://github.com/Lateef-ZHOU/LiDAR_DataACQ-simonly) 通过固定版本装配这些入口，负责 JSON、命令行、设备就绪后启动和系统信号取消。窗口隐藏仍保留历史，不触发实际绘制。测试创建两套真实页面并检查后端唯一性、销毁和精简分区；跨设备的三任务采样与保存由应用集成测试覆盖。

2026-11-02：独立协程退出时由自身排队通知 TaskBase 回读完成状态，覆盖设备主动关闭导致的取消；不借用设备通用状态信号推断任务结束，不改编排、锁与消费链。

## 源码目录（2026-11-03）

公开文件直接放在 `include/`，引用时使用文件名，不再携带原单仓库的业务目录前缀。CMake 目标名称、C++ 类型和 QML 模块名保持不变；使用方须同步更新头文件路径及固定依赖版本。

目录整理验证（2026-11-03）：Windows：公开头独立编译、页面测试 1/1 及 simonly 运行测试通过。本轮通过本地源码覆盖验证，固定依赖同步更新到本地提交；尚未推送，不代表已验证远端拉取。

2026-11-05 表单统一（用户定）：蛇形计划使用 UICore 宽标签预设，业务参数与独立计划保持。 公共视觉规格由 UICore 提供，不增加业务状态。Windows 主程序增量构建、八秒离屏冒烟通过；固定依赖在本地更新，暂不推送。


2026-11-05：同步 TaskCore/UICore 公共视觉依赖；本仓库计划页与独立协程不变。本地源码覆盖完成主程序 Windows 增量构建及八秒冒烟，未推送。
