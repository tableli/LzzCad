# LzzCad

一个基于 Qt 和 OpenCASCADE 开发的开源 CAD 软件，集成 AI 智能助手。

## ✨ 功能特性

### 🤖 AI 智能助手
- ✅ **AI Chat 面板** - 内置 AI 聊天界面，支持自然语言交互
- ✅ **文件操作** - 通过 AI 命令打开 STEP/BREP/IGES 模型文件
- ✅ **模型保存** - 通过 AI 命令保存当前模型
- ✅ **草图绘制** - AI 根据描述自动绘制点、线、圆、圆弧
- ✅ **上下文感知** - AI 了解当前模型状态，提供精准建议
- ✅ **DeepSeek 集成** - 使用 DeepSeek 大模型提供智能服务

### 📐 草图绘制
- ✅ **基础草图** - 支持绘制点（Point）、线（Line）、圆（Circle）、圆弧（Arc）
- ✅ **交互式绘制** - 鼠标拖动确定尺寸，实时预览
- ✅ **ESC 退出** - 按 ESC 键退出草图模式
- ✅ **颜色区分** - 不同草图元素使用不同颜色（点-红、线-蓝、圆-紫、圆弧-橙）

### 🧊 基本体建模
- ✅ **Box（长方体）** - 点击对角两点确定底面，输入高度
- ✅ **Sphere（球体）** - 点击确定球心，拖动确定半径
- ✅ **Cylinder（圆柱）** - 点击确定圆心，拖动确定半径，输入高度
- ✅ **Cone（圆锥）** - 点击确定底面圆心，拖动确定半径，输入高度
- ✅ **交互式创建** - 实时预览模型，支持负数高度（向下拉伸）
- ✅ **自动命名** - 自动生成名称（Box_1, Sphere_1, ...）

### 🔧 特征操作
- ✅ **Extrude（拉伸）** - 将草图沿 Z 轴拉伸生成实体/曲面
  - 支持 Face 模式（实心）和 Wire 模式（曲面）
  - 支持正负高度值
  - 封闭草图验证
- ✅ **Revolve（旋转）** - 将草图绕轴旋转生成实体/曲面
  - 支持 X/Y/Z 轴选择
  - 自定义旋转角度（0-360度）
  - 支持 Face 模式和 Wire 模式
- ✅ **Sweep（扫掠）** - 将截面沿路径扫掠生成实体/曲面
  - 选择截面轮廓和扫掠路径
  - 支持 Face 模式和 Wire 模式
  - 封闭截面验证

### 📁 文件操作
- ✅ **STEP 导入/导出** - 支持 `.step` / `.stp` 格式
- ✅ **BREP 导入/导出** - 支持 `.brep` / `.brp` 格式
- ✅ **IGES 导入/导出** - 支持 `.iges` / `.igs` 格式
- ✅ **颜色设置** - STEP 文件加载时使用黄色显示

### 🎨 属性面板
- ✅ **颜色修改** - 点击颜色按钮自定义模型颜色
- ✅ **形状检查器** - 显示几何信息（顶点数、边数、面数等）
- ✅ **变换面板** - 支持平移和旋转操作
- ✅ **名称编辑** - 修改模型名称
- ✅ **图层信息** - 显示图层属性

### 🌳 模型树
- ✅ **动态更新** - 形状添加/删除时自动更新
- ✅ **可见性切换** - 点击图标切换模型显示/隐藏
- ✅ **选择高亮** - 选中节点时高亮对应模型
- ✅ **双击编辑** - 双击修改名称

### 👁️ 视图控制
- ✅ **六面视图** - Top/Bottom/Left/Right/Front/Back
- ✅ **旋转** - 中键拖动旋转视图
- ✅ **平移** - Pan 模式下中键拖动平移
- ✅ **缩放** - 鼠标滚轮缩放
- ✅ **Fit All** - 自动调整视图显示所有模型
- ✅ **着色/线框** - 切换显示模式
- ✅ **ViewCube** - 右上角方向指示立方体

### ↩️ Undo/Redo
- ✅ **命令模式** - 基于 Command 模式实现
- ✅ **Undo** - 撤销上一步操作
- ✅ **Redo** - 重做已撤销的操作
- ✅ **复合命令** - 支持批量操作的撤销

### 📋 Help 菜单
- ✅ **Guide（指南）** - 点击后在日志面板显示操作提示
- ✅ **Bug Report（Bug报告）** - 指引用户在 GitHub 提交 issues
- ✅ **About（关于）** - 显示版本信息、功能列表、联系方式

### 🔮 计划中
- 🔲 Loft（放样）- 多截面放样功能
- 🔲 布尔运算 - Union、Intersection、Cut
- 🔲 倒角/圆角 - Chamfer 和 Fillet
- 🔲 阵列 - 矩形阵列和圆形阵列
- 🔲 草图约束 - 水平/垂直/平行/垂直约束
- 🔲 尺寸标注 - 长度、角度、半径标注
- 🔲 CAM 功能 - 刀路生成、G代码输出
- 🔲 装配设计 - 多零件装配和配合约束

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

### 文件操作

| 命令 | 说明 | 示例 |
|------|------|------|
| `open_file` | 打开模型文件 | `打开 C:/models/part.step` |
| `save_file` | 保存模型 | `保存到 C:/models/my_model.step` |
| `clear` | 清除所有形状 | `清空所有模型` |

### AI 草图绘制

AI 可以根据你的描述自动绘制草图，支持以下绘图命令：

| 命令 | 说明 | 参数 |
|------|------|------|
| `draw_point` | 绘制点 | `x`, `y` - 坐标 |
| `draw_line` | 绘制直线 | `x1`, `y1`, `x2`, `y2` - 起点和终点坐标 |
| `draw_circle` | 绘制圆 | `cx`, `cy` - 圆心坐标, `radius` - 半径 |
| `draw_arc` | 绘制圆弧 | `cx`, `cy` - 圆心坐标, `radius` - 半径, `start_angle`, `sweep_angle` - 起始角度和扫掠角度 |
| `draw_polyline` | 绘制多段线 | `points` - 点数组 |
| `draw_polygon` | 绘制多边形 | `points` - 点数组 |

3. 也可以直接聊天，AI 会根据上下文提供建模建议

**示例对话：**
```
You: 帮我打开一个 STEP 文件
AI: 请提供文件路径，例如：打开 C:/models/part.step

You: 打开 E:/MyGithub/LzzCad/test/step/qiur20.step
AI: 正在打开文件...
System: Opening file E:/MyGithub/LzzCad/test/step/qiur20.step
```

**AI 绘制草图示例：**
```
You: 画一个笑脸
AI: 正在绘制笑脸...
System: 绘制脸 (Circle: center (0,0), radius 50)
System: 绘制左眼 (Circle: center (-20,15), radius 8)
System: 绘制右眼 (Circle: center (20,15), radius 8)
System: 绘制微笑嘴巴 (Arc: center (0,-10), radius 25, start 210°, sweep 120°)
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
│   └── Command.h      # 命令基类和子类
├── Model/             # 数据模型
│   ├── GeometryModel.h/cpp
├── View/              # 视图层
│   ├── PropertyPanel.h/cpp
├── ViewModel/         # 视图模型
│   ├── OccViewModel.h/cpp
├── image/             # 图标资源
│   ├── logo/          # 软件图标
│   ├── modeltree/     # 模型树图标
│   └── ribbon/        # Ribbon 按钮图标
├── test/              # 测试文件
│   └── step/          # STEP 测试模型
├── LzzCad.cpp/h       # 主窗口（含 AI 功能）
├── view.cpp/h         # OCC 视图（含建模逻辑）
└── main.cpp           # 入口函数
```

## 🚀 快速开始

1. 编译并运行 LzzCad
2. **配置 AI（可选）**: 点击 Help → API Settings 输入 DeepSeek API Key
3. **绘制草图**: 使用 Draw 选项卡中的工具绘制点、线、圆、圆弧
4. **创建基本体**: 使用 Model 选项卡中的工具创建 Box、Sphere、Cylinder、Cone
5. **特征操作**: 使用 Extrude、Revolve、Sweep 创建复杂模型
6. **编辑属性**: 在右侧属性面板中修改颜色、变换等属性
7. **管理模型**: 在左侧模型树中查看和管理模型结构
8. **AI 交互**: 在底部 AI Chat 面板中与 AI 助手交互

## 📄 许可证

本项目采用 GPL-3.0 许可证。详见 [LICENSE](LICENSE) 文件。

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

## 📧 联系方式

- **Email**: 2521403134@qq.com
- **GitHub**: [https://github.com/tableli/LzzCad](https://github.com/tableli/LzzCad)

---

**Made with ❤️ by tableli**
