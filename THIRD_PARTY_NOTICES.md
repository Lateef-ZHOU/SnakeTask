# 第三方依赖

本仓库源码使用 MIT；所链接依赖不因此重新授权。

- Qt 6：按所用模块的 LGPL/GPL 或商业许可使用和分发；本模块使用 Qt Core/Gui/Quick/Qml 及 ShaderTools 构建工具，私有 GUI API 要求版本对齐。分发 Qt 时应保留许可、通知并满足对应授权条件。
- HandleCore、DeviceCore、TaskCore、UICore、ChartCore、Algorithm：MIT，各自的许可证与第三方声明随所固定提交保留。
- 串口依赖 libserialport：LGPL-3.0-or-later；仅串口官邸需要。动态/静态分发应按依赖许可履行义务。
- Linux libpcap：BSD 类许可，保留其版权和声明。
- Windows Npcap：SDK 与驱动许可各自适用；免费许可不等于可任意再分发驱动。未取得相应授权时，由使用者从官方安装，不随产品打包。

依赖的完整文本及版本边界以各库固定提交和实际安装版本为准；HandleCore 的 THIRD_PARTY_NOTICES.md 提供串口与抓包依赖链接。
