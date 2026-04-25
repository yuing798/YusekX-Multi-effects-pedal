
# 综合效果器
这是一个基于 **JUCE 7** 框架开发的 C++ 综合效果器。本项目采用现代 CMake 构建方式，特别针对 **Windows MSVC** 环境进行了优化。

**本项目目前处于个人实验阶段，暂不接受 PR，谢谢配合**

---

## 功能介绍

目前已实现以下核心功能：
1、效果器页面初始化

2、基本延迟效果器

3、基本tremolo颤音效果器

4、正弦空间环绕效果器

## 项目框架
pluginEditor文件：放置和UI线程相关的内容

pluginProcessor文件：放置和DSP音频线程相关的内容

Utils/:放置所有的工具函数，工具类，工具常量

plugins/:放置所有的单块效果器

copilot_docs/:给ai写代码时的上下文，当然你也可以看

images/:顾名思义，图片

效果器设计指南请查阅：[设计指南](https://github.com/yuing798/effect-pedal-develop-guide)

## 🛠 开发环境要求 (Prerequisites)
为了确保编译成功，你的电脑需要配置以下环境：

- **操作系统**: Windows 10/11
- **编译器**: Visual Studio 2022 (必须安装 **“使用 C++ 的桌面开发”** 工作负载)
  - *注意：本项目强制使用 cl.exe (MSVC)，不建议使用 MinGW 或 Clang。*
- **构建工具**: CMake (3.22 或更高版本)
- **生成器**: Ninja (推荐，已在 Preset 中预设)

---

## 🚀 快速开始 (How to Build)
本项目提供了 CMake Presets 预设，推荐使用以下两种方式之一进行编译：

### 方法 A：使用 VS Code (推荐)

1. 确保已安装插件：`C/C++` 和 `CMake Tools`。
2. 在 VS Code 底部状态栏点击 **CMake: [Select Configure Preset]**。
3. 在弹出的列表中选择 **GitHub Build (Auto JUCE)**。
  - *此选项会自动从官方 GitHub 下载 JUCE 框架（无需手动配置路径）。*
4. 点击底部的 **Build** (⚙ 生成) 进行编译。

5. 可执行文件路径：build\预设名\Multi_effects_processor_artefacts\Debug\Standalone\Multi_effects_processor.exe
6. 请认真查看CMakeLists.txt中的以下内容
   ![alt text](images\image.png)
   
   选择你想要生成的形式，如果是standalone,则生成独立应用，如果是VST3,则生成VST3插件格式文件，放置到数字音频工作站（DAW）中的指定路径即可使用，如果是AU插件，则需要MACOS系统，生成的文件格式都可以在build\预设名\Multi_effects_processor_artefacts\Debug下面找到

### 方法 B：使用命令行 (Developer Command Prompt)
请务必在 **Visual Studio 的 Developer Command Prompt** 中运行以下命令，以确保环境变量中包含 `cl.exe`：

```bash
# 1. 克隆项目
git clone <你的项目地址>
cd <该项目名>

# 2. 配置项目 (自动下载 JUCE)
cmake --preset github-public

# 3. 编译项目
cmake --build --preset debug

```
---

## 📂 关于 JUCE 框架的配置
本项目支持两种加载 JUCE 的方式，通过 `USE_FETCHCONTENT` 开关控制：

1. **自动模式 (USE_FETCHCONTENT=ON)**:
  - **适用人群**：初次克隆项目的外部开发者。
  - **行为**：CMake 会自动下载 JUCE 7.0.5 源码到 `build/_deps` 目录下。
  - **预设**：直接选择 `github-public` 预设即可。
2. **本地模式 (USE_FETCHCONTENT=OFF)**:
  - **适用人群**：本人，以及本地计算机有安装juce的人士。
  - **行为**：链接到本地已编译好的 JUCE 路径。
  - **预设**：`My Local Dev (Manual JUCE)`。

---

## 📝 开发者备注

- **编译器一致性**：由于 JUCE 对音频底层处理的特殊要求，本项目在 `CMakePresets.json` 中锁定了 `cl` 编译器和 `v143` 工具链，以保证音频算法的稳定性。
- **清理缓存**：如果切换了预设后编译报错，建议删除 `build` 文件夹并重新进行 Configure。

---

## 贡献指南

个人编码风格以及ai编码规则已经放在copilot_docs/文件夹中


### 📅 TODO List (后续计划)

1、自主调节音色链顺序功能

2、合唱chorus效果器

---


