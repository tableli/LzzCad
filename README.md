# LzzCad

一个基于 Qt 和 OpenCASCADE 开发的开源 CAD 软件，集成 AI 智能助手。

## ✨ 功能特性

### 🤖 AI 智能助手
- ✅ **AI Chat 面板** - 内置 AI 聊天界面，支持自然语言交互
- ✅ **文件操作** - 通过 AI 命令打开 STEP/BREP/IGES 模型文件
- ✅ **模型保存** - 通过 AI 命令保存当前模型
- ✅ **上下文感知** - AI 了解当前模型状态，提供精准建议
- ✅ **DeepSeek 集成** - 使用 DeepSeek 大模型提供智能服务

### 📐 建模功能
- ✅ **模型树** - 动态显示模型结构，支持可见性切换
- ✅ **STEP 文件导入** - 支持导入 STEP 格式的 3D 模型
- ✅ **基础草图绘制** - 支持绘制点、线、圆、圆弧
- ✅ **属性面板** - 查看和编辑模型属性（形状检查器、变换、草图属性）
- ✅ **形状变换** - 支持平移和旋转操作
- ✅ **形状高亮** - 选中模型树节点时高亮显示对应形状
- ✅ **形状删除** - 支持删除选中的形状
- ✅ **视图控制** - 支持旋转、平移、缩放视图

### 🔮 计划中
- 🔲 模型保存功能
- 🔲 撤销/重做操作
- 🔲 更多草图工具（矩形、椭圆、样条曲线等）
- 🔲 实体建模功能（拉伸、旋转、扫掠等）
- 🔲 布尔运算（并集、交集、差集）
- 🔲 尺寸标注

## 🤖 AI 智能助手使用指南

### 配置 API Key

1. 点击菜单栏 **Help** → **API Settings**
2. 输入你的 DeepSeek API Key（可在 [DeepSeek 官网](https://platform.deepseek.com/) 获取）
3. 点击确定保存

### 或通过配置文件

在项目根目录创建 `deepseek_api.txt` 文件，内容为你的 API Key：

```
your_api_key_here
```

### 使用 AI 助手

1. 在底部的 **AI Chat** 面板中输入问题
2. AI 支持以下工具命令：

| 命令 | 说明 | 示例 |
|------|------|------|
| `open_file` | 打开模型文件 | `打开 C:/models/part.step` |
| `save_file` | 保存模型 | `保存到 C:/models/my_model.step` |
| `clear` | 清除所有形状 | `清空所有模型` |

3. 也可以直接聊天，AI 会根据上下文提供建模建议

**示例对话：**
```
You: 帮我打开一个 STEP 文件
AI: 请提供文件路径，例如：打开 C:/models/part.step

You: 打开 E:/MyGithub/LzzCad/test/step/qiur20.step
AI: 正在打开文件...
System: Opening file E:/MyGithub/LzzCad/test/step/qiur20.step
```

## 🛠️ 技术栈

- **框架**: Qt 5.14
- **几何引擎**: OpenCASCADE 7.6+
- **UI 组件**: SARibbon（Office 风格 Ribbon 界面）
- **AI 模型**: DeepSeek Chat
- **语言**: C++17

## 📦 编译环境

### 依赖要求
- Qt 5.14（必须）
- OpenCASCADE 7.6 或更高版本
- Visual Studio 2019 或更高版本（Windows）

### 编译步骤

1. **安装 Qt 5.14**
   - 下载并安装 Qt 5.14
   - 确保安装了 `msvc2019_64` 组件

2. **安装 OpenCASCADE**
   - 从 [OpenCASCADE 官网](https://www.opencascade.com/content/download) 下载最新版本
   - 解压到合适的目录

3. **配置项目**
   - 使用 Qt Creator 打开 `LzzCad.sln` 或 `LzzCad.pro`
   - 在项目设置中配置 OpenCASCADE 的包含目录和库目录

4. **编译运行**
   - 选择 Debug 或 Release 配置
   - 点击编译按钮

## 📁 项目结构

```
LzzCad/
├── 3rdparty/          # 第三方库
│   └── SARibbon/      # Ribbon UI 组件
├── Command/           # 命令模式实现
├── Model/             # 数据模型
│   ├── GeometryModel.h/cpp
├── View/              # 视图层
│   ├── PropertyPanel.h/cpp
├── ViewModel/         # 视图模型
│   ├── OccViewModel.h/cpp
├── image/             # 图标资源
├── test/              # 测试文件
├── LzzCad.cpp/h       # 主窗口（含 AI 功能）
├── view.cpp/h         # OCC 视图
└── main.cpp           # 入口函数
```

## 🚀 快速开始

1. 编译并运行 LzzCad
2. **配置 AI（可选）**: 点击 Help → API Settings 输入 DeepSeek API Key
3. 使用 Ribbon 工具栏中的工具绘制草图或导入模型
4. 在模型树中查看和管理模型结构
5. 使用属性面板编辑模型属性和变换
6. 在 AI Chat 面板中与 AI 助手交互

## 📄 许可证

本项目采用 GPL-3.0 许可证。详见 [LICENSE](LICENSE) 文件。

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

## 📧 联系方式

如有问题或建议，请通过 GitHub Issues 联系。