# LzzCad

一个基于 Qt 和 OpenCASCADE 开发的开源 CAD 软件。

## ✨ 功能特性

### 已实现
- ✅ **模型树** - 动态显示模型结构，支持可见性切换
- ✅ **STEP 文件导入** - 支持导入 STEP 格式的 3D 模型
- ✅ **基础草图绘制** - 支持绘制点、线、圆、圆弧
- ✅ **属性面板** - 查看和编辑模型属性（形状检查器、变换、草图属性）
- ✅ **形状变换** - 支持平移和旋转操作
- ✅ **形状高亮** - 选中模型树节点时高亮显示对应形状
- ✅ **形状删除** - 支持删除选中的形状
- ✅ **视图控制** - 支持旋转、平移、缩放视图

### 计划中
- 🔲 模型保存功能
- 🔲 撤销/重做操作
- 🔲 更多草图工具（矩形、椭圆、样条曲线等）
- 🔲 实体建模功能（拉伸、旋转、扫掠等）
- 🔲 布尔运算（并集、交集、差集）
- 🔲 尺寸标注

## 🛠️ 技术栈

- **框架**: Qt 5.15+
- **几何引擎**: OpenCASCADE 7.6+
- **UI 组件**: SARibbon（Office 风格 Ribbon 界面）
- **语言**: C++17

## 📦 编译环境

### 依赖要求
- Qt 5.15 或更高版本
- OpenCASCADE 7.6 或更高版本
- Visual Studio 2019 或更高版本（Windows）

### 编译步骤

1. **安装 Qt**
   - 下载并安装 Qt 5.15 或更高版本
   - 确保安装了 `msvc2019_64` 或 `msvc2022_64` 组件

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
├── LzzCad.cpp/h       # 主窗口
├── view.cpp/h         # OCC 视图
└── main.cpp           # 入口函数
```

## 🚀 快速开始

1. 编译并运行 LzzCad
2. 使用 Ribbon 工具栏中的工具绘制草图或导入模型
3. 在模型树中查看和管理模型结构
4. 使用属性面板编辑模型属性和变换

## 📄 许可证

本项目采用 GPL-3.0 许可证。详见 [LICENSE](LICENSE) 文件。

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

## 📧 联系方式

如有问题或建议，请通过 GitHub Issues 联系。