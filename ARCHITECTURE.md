# LzzCad 项目架构说明

## 一、项目概述

LzzCad 是一个基于 **Qt 5.14.2** 和 **OpenCASCADE 7.x** 的 CAD（计算机辅助设计）软件，采用 **MVVM** 架构模式。支持 3D 模型显示与交互、多种 CAD 文件格式读写（STEP/BREP/IGES）、2D 草图绘制、视图操作（旋转/平移/缩放/标准视角）、AI 辅助绘图功能以及基于命令模式的 Undo/Redo 系统。

---

## 二、技术栈

### 2.1 主要框架和库

| 技术 | 版本 | 作用 |
|------|------|------|
| **Qt** | 5.14.2 (msvc2017_64) | 主框架，提供 UI 组件、信号槽机制、网络请求等 |
| **OpenCASCADE** | 7.x | 3D 几何建模核心和可视化引擎 |
| **SARibbon** | 2.9.0 (开源) | Office 风格的 Ribbon 界面组件，支持多种主题 |
| **OpenSSL** | 1.1.1w | HTTPS 加密通信（AI API 调用） |
| **Visual Studio** | 2019 (v142) | 开发环境和编译器 |

### 2.2 Qt 模块

| 模块 | 作用 |
|------|------|
| Qt5Core | 核心功能（信号槽、容器类、JSON 处理等） |
| Qt5Gui | GUI 基础组件 |
| Qt5Widgets | 界面控件（按钮、对话框、树、文本编辑等） |
| Qt5Network | 网络功能（HTTP POST/GET 请求） |

### 2.3 OpenCASCADE 核心模块

| 模块 | 作用 |
|------|------|
| TKernel | 基础类型（gp_Pnt, gp_Dir, Quantity_Color 等） |
| TKMath | 数学计算 |
| TKBRep | BRep 数据结构（TopoDS_Shape, BRepBuilderAPI 等） |
| TKTopAlgo | 拓扑算法 |
| TKPrim | 基本几何体（BRepPrimAPI_MakeBox 等） |
| TKSTEP | STEP 文件读写（STEPControl_Reader/Writer） |
| TKIGES | IGES 文件读写（IGESControl_Reader/Writer） |
| TKV3d | 3D 可视化（V3d_View, V3d_Viewer, Graphic3d 等） |
| TKAIS | 交互选择框架（AIS_Shape, AIS_InteractiveContext 等） |

---

## 三、架构设计

### 3.1 MVVM 架构

```
+------------------------------------------------------------------+
|                              View                                 |
|  +----------------------+  +----------------------------------+  |
|  |  LzzCad (主窗口)      |  |  OccView (OCC 3D 视图)           |  |
|  |  - SARibbon 界面     |  |  - V3d_View 渲染                  |  |
|  |  - DockWidget 布局    |  |  - 鼠标交互（选择/草图）           |  |
|  |  - AI 对话面板        |  |  - 视图操作 + ViewCube            |  |
|  |  - 菜单/快捷键        |  |  - 草图绘制（点/线/圆/弧）        |  |
|  +----------+-----------+  +--------------+-------------------+  |
|             |                              |                       |
|             +----------+-------------------+                       |
|                        | 信号槽连接                                |
+------------------------------------------------------------------+
|                         ViewModel                                 |
|  +-------------------------------------------------------------+  |
|  |                     OccViewModel                            |  |
|  |  - 业务逻辑处理                                              |  |
|  |  - Q_PROPERTY (statusText, isShaded)                        |  |
|  |  - 视图操作（fitAll/rotate/pan/zoom）                         |  |
|  |  - 标准视角（viewTop/Bottom/Left/Right/Front/Back）            |  |
|  |  - 显示模式（shaded/wireframe 切换）                          |  |
|  |  - 模型操作（addBox/clearAllShapes）                          |  |
|  |  - 草图添加（addPoint/addLine/addCircle/addArc）              |  |
|  |  - 文件操作（openFile/saveFile）                              |  |
|  +--------------------+----------------------------------------+  |
|                       | 方法调用                                 |
+------------------------------------------------------------------+
|                          Model                                    |
|  +-------------------------------------------------------------+  |
|  |                   GeometryModel                             |  |
|  |  - std::vector<Handle(AIS_Shape)> 存储                       |  |
|  |  - 文件读写（STEP/BREP/IGES 三种格式）                       |  |
|  |  - 添加/删除/清空形状                                        |  |
|  +-------------------------------------------------------------+  |
+------------------------------------------------------------------+
|                        Command Layer                              |
|  +-------------------------------------------------------------+  |
|  |                 CommandManager                               |  |
|  |  - Undo/Redo 命令管理                                        |  |
|  |  - 命令栈（最大 50 步）                                       |  |
|  |  - 支持：DrawPoint/DrawLine/DrawCircle/Delete                |  |
|  +-------------------------------------------------------------+  |
+------------------------------------------------------------------+
```

### 3.2 MVVM 组件交互流程

```
用户操作 -> LzzCad (View) 槽函数 -> 调用 OccViewModel 方法
    OccViewModel 更新状态 + 发射信号
    OccView 接收信号 -> OCC 渲染更新
    
或者：用户操作 -> LzzCad -> 直接调用 OccView 方法 -> OCC 渲染更新
    (对于不需要 ViewModel 的简单操作)
```

### 3.3 各层职责与文件对应

| 层级 | 关键类 | 文件位置 | 核心职责 |
|------|--------|----------|----------|
| Model | GeometryModel | Model/GeometryModel.h/cpp | 几何数据管理、文件 IO |
| ViewModel | OccViewModel | ViewModel/OccViewModel.h/cpp | 业务逻辑编排、状态管理 |
| View | LzzCad | LzzCad.h/cpp | 主窗口、Ribbon、Dock、AI 对话 |
| View | OccView | view.h/cpp | OCC 3D 视图、鼠标交互、草图绘制 |
| Command | CommandManager | Command/Command.h | Undo/Redo 命令管理 |

---

## 四、项目文件结构

```
LzzCad/
  main.cpp                        入口文件
  LzzCad.h/cpp                    主窗口（Ribbon + Dock + AI）
  LzzCad.ui / LzzCad.qrc          UI / 资源文件
  LzzCad.sln / LzzCad.vcxproj     解决方案/项目文件
  ARCHITECTURE.md                 架构文档
  deepseek_api.txt                DeepSeek API Key
  Model/GeometryModel.h/cpp       几何数据模型
  ViewModel/OccViewModel.h/cpp    ViewModel 业务逻辑
  Command/Command.h               命令模式（Undo/Redo）
  view.h/cpp                      OccView（3D 视图 + 草图）
  3rdparty/SARibbon/              Ribbon 组件（单头文件）
  image/logo/lzz.png              应用程序图标
  image/ribbon/                   工具栏图标（~80 个 PNG）
  test/step/qiur20.step           测试文件
  x64/Debug/                      构建输出
```

---

## 五、核心模块详解

### 5.1 主窗口 (LzzCad)

**继承关系**: `SARibbonMainWindow` -> `QMainWindow`

**初始化流程**:
```
LzzCad 构造函数
  +-- ui.setupUi(this)              加载 UI
  +-- setWindowTitle + resize        1600x1200
  +-- createRibbon()                 7 个 Category
  +-- setupWindowIcon()              logo/lzz.png
  +-- setRibbonTheme()               Office 2021 Blue
  +-- createCentralWidget()          创建 OCC 视图
  +-- createDockWidgets()            3 个 Dock
  +-- statusBar() 设置               状态栏 + 坐标
  +-- AI 初始化                      QNetworkAccessManager + API
  +-- CommandManager 初始化          Undo/Redo 管理器
```

**Ribbon 界面（7 个 Category）**：

| Category | 面板 | 功能按钮 |
|----------|------|----------|
| File | File / Edit / Window | Open, Save, Save As, Undo, Redo, Close, Exit |
| Sketch | Basic / Curve | Point, Line, Circle, Arc, Polyline, Spline, Ellipse, Rectangle |
| Model | Primitives / Features / Modify / Intersect | Box, Cylinder, Sphere, Cone, Extrude, Revolve, Sweep, Loft, Chamfer, Fillet, Shell, Pocket, Draft, Cut, Intersect, Union, Contour |
| View | View / Orientations / Display / Camera | Fit All, Pan, Zoom, Rotate, Top, Bottom, Front, Back, Left, Right, Shaded, Wireframe, Window, Capture |
| Tools | Measure / Analyze / Reverse / Mesh | Distance, Angle, Area, Volume, Check Geometry, Mass Properties, Reverse Engineer, Mesh, Fill, Delete |
| CAM | CAM / Import / Project | G-Code, Simulate, Drilling, Point Cloud, Project Along Direction, To Plane |
| Help | About / AI | About, Guide, API Settings, Bug Report |

**Dock 面板**：

| Dock | 位置 | 组件 | 说明 |
|------|------|------|------|
| Model Tree | 左侧 | QTreeWidget | 模型结构树 |
| Properties | 右侧 | QTreeWidget (2 列) | 属性面板 |
| Log & AI | 底部（max 200px） | 50% Log + 50% AI Chat | 日志 + AI 对话 |

**关键成员变量**：
```cpp
// UI 组件
QTreeWidget* m_modelTree;           // 模型树
QTreeWidget* m_propertyTree;        // 属性面板
QTextEdit* m_logEdit;               // 日志窗口
QLabel* m_positionLabel;            // 状态栏坐标显示

// AI 聊天成员
QTextEdit* m_chatHistory;           // 聊天历史
QLineEdit* m_chatInput;             // 输入框
QPushButton* m_sendButton;          // 发送按钮
QNetworkAccessManager* m_networkManager; // 网络管理器
QString m_apiKey;                   // API Key
QString m_apiUrl;                   // API URL
bool m_isProcessing;                // 是否正在处理请求

// MVVM 组件
GeometryModel* m_geometryModel;     // 数据模型
OccViewModel* m_occViewModel;       // 视图模型
OccView* m_occView;                 // OCC 视图

// 命令管理器
CommandManager* m_commandManager;   // Undo/Redo 管理器
```

**核心方法**：

| 方法 | 说明 |
|------|------|
| `createRibbon()` | 创建 Ribbon 界面 |
| `createCentralWidget()` | 创建中心 OCC 视图 |
| `createDockWidgets()` | 创建三个 Dock 面板 |
| `loadIcon()` | 加载图标（自动缩放至 24px） |
| `onSendMessage()` | 发送 AI 消息 |
| `onApiResponse()` | 处理 AI 响应 |
| `parseAndExecuteTool()` | 解析并执行 AI 返回的工具调用 |
| `processToolCall()` | 处理单个工具调用 |
| `extractJsonFromText()` | 从文本中提取 JSON |
| `onUndo()` / `onRedo()` | Undo/Redo 操作 |
| `onDeleteSelected()` | 删除选中对象（支持 Undo） |

---

### 5.2 OCC 3D 视图 (OccView)

**继承**: `QWidget`

**初始化**:
```
init()
  +-- OpenGl_GraphicDriver（单例）
  +-- WNT_Window
  +-- V3d_Viewer + V3d_View
  +-- AIS_InteractiveContext
  +-- 光照（环境光 + 3 方向光）
  +-- 背景渐变 (SLATEGRAY3->4)
  +-- Trihedron（左下角）
  +-- ViewCube（右上角）
```

**10 种操作模式 (CurrentAction3d)**：
```cpp
enum CurrentAction3d
{
    CurAction3d_Nothing,
    CurAction3d_DynamicZooming,
    CurAction3d_WindowZooming,
    CurAction3d_DynamicPanning,
    CurAction3d_GlobalPanning,
    CurAction3d_DynamicRotation,
    CurAction3d_Sketch_DrawPoint,
    CurAction3d_Sketch_DrawLine,
    CurAction3d_Sketch_DrawCircle,
    CurAction3d_Sketch_DrawArc
};
```

**鼠标交互**：

| 事件 | 操作 |
|------|------|
| 左键拖拽 | 框选 |
| 左键单击 | 单选 |
| 中键拖拽 | 旋转/缩放/平移（按模式） |
| 中键单击 | 居中平移 |
| 右键 | popup（空） |
| 移动 | 高亮 + 坐标信号 |
| 滚轮 | 缩放 |
| Ctrl+左键 | 多选 |
| Esc | 取消草图 |
| Delete | 删除选中 |

**草图绘制**：
- Point: 点击 1 次放置点
- Line: 点击 2 次（起点 → 终点）
- Circle: 点击 2 次（圆心 → 半径点）
- Arc: 点击 3 次（圆心 → 起点 → 终点），第一个点显示红色标记

**坐标转换**：屏幕坐标 → 射线 → 与 Z=0 平面求交

**关键成员变量**：
```cpp
// OCC 核心对象
Handle(V3d_Viewer) myViewer;                    // 视图管理器
Handle(AIS_InteractiveContext) myContext;       // 交互上下文
Handle(V3d_View) myView;                        // 视图对象
Handle(AIS_Manipulator) myManipulator;          // 操作器
Handle(AIS_Shape) mySelectedShape;              // 选中的形状

// 交互状态
bool isManipulatorActive = false;               // 操作器激活状态
bool isMousePressed = false;                    // 鼠标按下状态
Standard_Integer myXmin, myYmin, myXmax, myYmax; // 拖拽矩形范围
CurrentAction3d myCurrentMode;                  // 当前操作模式
Standard_Boolean myDegenerateModeIsOn;          // 退化模式
QRubberBand* myRectBand;                        // 框选矩形

// 草图绘制状态
int m_sketchClickCount;                         // 点击计数
Handle(AIS_Shape) m_previewShape;               // 预览形状
gp_Pnt m_sketchPoint1, m_sketchPoint2, m_sketchPoint3; // 草图点
bool m_sketchReverseArc;                        // 圆弧方向

// 引用
OccViewModel* m_viewModel = nullptr;            // 视图模型引用
CommandManager* m_commandManager = nullptr;     // 命令管理器引用
```

**核心方法**：

| 方法 | 说明 |
|------|------|
| `init()` | 初始化 OCC 视图 |
| `displayShape()` | 显示形状 |
| `clearAllShapes()` | 清空所有形状 |
| `startSketchPointMode()` / `startSketchLineMode()` / `startSketchCircleMode()` / `startSketchArcMode()` | 启动草图模式 |
| `stopSketchMode()` | 停止草图模式 |
| `convertScreenToWorld()` | 屏幕坐标转世界坐标 |
| `updateSketchPreview()` | 更新草图预览 |
| `finishSketchShape()` | 完成草图绘制 |
| `resetSketchState()` | 重置草图状态 |
| `drawPoint()` / `drawLine()` / `drawCircle()` / `drawArc()` | AI 直接绘制方法 |
| `drawPolyline()` / `drawPolygon()` | 绘制多段线/多边形 |
| `eraseSelected()` | 删除选中对象 |
| `getSelectedObjects()` | 获取选中对象列表 |

---

### 5.3 GeometryModel

**数据存储**：
```cpp
std::vector<Handle(AIS_Shape)> m_shapes;
```

**核心方法**：

| 方法 | 说明 |
|------|------|
| `addShape()` | 添加形状到列表 |
| `removeShape()` | 从列表移除形状 |
| `getShapes()` | 获取所有形状（const 引用） |
| `shapeCount()` | 获取形状数量 |
| `clearAll()` | 清空所有形状 |
| `readStepFile()` | 读取 STEP 文件 |
| `readBrepFile()` | 读取 BREP 文件 |
| `readIgesFile()` | 读取 IGES 文件 |
| `writeStepFile()` | 写入 STEP 文件 |
| `writeBrepFile()` | 写入 BREP 文件 |
| `writeIgesFile()` | 写入 IGES 文件 |

**文件读写逻辑**：
- 读取时：解析文件 → 获取 TopoDS_Shape → 封装为 AIS_Shape → 添加到 m_shapes
- 写入时：将所有 AIS_Shape 合并为 TopoDS_Compound → 写入文件

---

### 5.4 OccViewModel

**Q_PROPERTY**：
```cpp
Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
Q_PROPERTY(bool isShaded READ isShaded NOTIFY isShadedChanged)
```

**核心方法**：

| 方法 | 说明 | 信号 |
|------|------|------|
| `fitAll()` | 适配全部对象 | `requestFitAll()` |
| `rotate()` | 切换旋转模式 | `requestRotate()` |
| `pan()` | 切换平移模式 | `requestPan()` |
| `zoom()` | 切换缩放模式 | `requestZoom()` |
| `viewTop()` / `viewBottom()` / `viewLeft()` / `viewRight()` / `viewFront()` / `viewBack()` | 切换标准视角 | `requestView*()` |
| `toggleShaded()` | 切换着色/线框 | `isShadedChanged()` + `requestSetShaded()` |
| `setShaded()` | 设置着色模式 | `isShadedChanged()` + `requestSetShaded()` |
| `addBox()` | 添加立方体 | `requestAddShape()` |
| `clearAllShapes()` | 清空所有形状 | `requestClearAll()` + `shapesCleared()` |
| `addPoint()` / `addLine()` / `addCircle()` / `addArc()` | 添加草图元素 | `requestAddShape()` |
| `openFile()` | 打开文件 | `fileOpened()` |
| `saveFile()` | 保存文件 | `fileSaved()` |

**信号桥梁模式**：
- View 调用 ViewModel 方法
- ViewModel 更新状态并发射 request* 信号
- OccView 通过槽函数接收信号并执行具体操作

---

### 5.5 命令模式 (Command System)

#### 5.5.1 Command 基类

```cpp
class Command
{
public:
    virtual ~Command() {}
    virtual void execute() = 0;      // 执行命令
    virtual void undo() = 0;         // 撤销命令
    virtual QString description() const = 0;  // 命令描述
};
```

#### 5.5.2 具体命令类

**DrawPointCommand**：
- 成员：`m_context`, `m_pointShape`, `m_point`, `m_color`, `m_executed`
- 构造：创建顶点形状，设置颜色
- execute()：显示形状
- undo()：隐藏形状

**DrawLineCommand**：
- 成员：`m_context`, `m_lineShape`, `m_p1`, `m_p2`, `m_color`, `m_executed`
- 构造：创建边形状（BRepBuilderAPI_MakeEdge）
- execute()：显示形状
- undo()：隐藏形状

**DrawCircleCommand**：
- 成员：`m_context`, `m_circleShape`, `m_center`, `m_radius`, `m_color`, `m_executed`
- 构造：创建圆形状（gp_Circ + BRepBuilderAPI_MakeEdge）
- execute()：显示形状
- undo()：隐藏形状

**DeleteCommand**：
- 成员：`m_context`, `m_deletedObjects`, `m_executed`
- 构造：保存待删除对象列表
- execute()：隐藏所有对象
- undo()：重新显示所有对象

**DisplayShapeCommand**：
- 成员：`m_context`, `m_shape`, `m_desc`, `m_executed`
- 用途：包装已显示的形状，支持 Undo
- execute()：显示形状
- undo()：隐藏形状

#### 5.5.3 CommandManager

**数据结构**：
```cpp
QStack<Command*> m_undoStack;   // 已执行命令栈（可撤销）
QStack<Command*> m_redoStack;   // 已撤销命令栈（可重做）
int m_maxDepth = 50;            // 最大撤销深度
```

**核心方法**：

| 方法 | 说明 |
|------|------|
| `executeCommand()` | 执行命令，推入 undo 栈，清空 redo 栈 |
| `canUndo()` | 是否可撤销 |
| `canRedo()` | 是否可重做 |
| `undo()` | 撤销：弹出 undo 栈 → 执行 undo() → 推入 redo 栈 |
| `redo()` | 重做：弹出 redo 栈 → 执行 execute() → 推入 undo 栈 |
| `undoDescription()` | 获取最近可撤销命令描述 |
| `redoDescription()` | 获取最近可重做命令描述 |
| `clear()` | 清空所有命令栈 |

**命令栈操作流程图**：
```
执行新命令：
  cmd->execute()
  push to undoStack
  limit undoStack to maxDepth
  clear redoStack

Undo：
  cmd = pop from undoStack
  cmd->undo()
  push to redoStack

Redo：
  cmd = pop from redoStack
  cmd->execute()
  push to undoStack
```

---

## 六、AI 集成

**API**: `https://api.deepseek.com/v1/chat/completions`
**Model**: `deepseek-chat`, Temperature: 0.7, Max Tokens: 2048

**28 个工具命令**：

| 类别 | 工具名称 | 参数 | 说明 |
|------|----------|------|------|
| 文件 | `open_file` | fileName | 打开文件 |
| 文件 | `save_file` | fileName | 保存文件 |
| 视图 | `clear_all` | - | 清空视图 |
| 视图 | `fit_all` | - | 适配全部 |
| 视图 | `view_top/bottom/front/back/left/right` | - | 标准视角 |
| 视图 | `shaded` / `wireframe` | - | 显示模式 |
| 交互草图 | `sketch_point` | - | 启动画点模式 |
| 交互草图 | `sketch_line` | - | 启动画线模式 |
| 交互草图 | `sketch_circle` | - | 启动画圆模式 |
| 交互草图 | `sketch_arc` | - | 启动画圆弧模式 |
| 交互草图 | `sketch_cancel` | - | 取消草图模式 |
| 程序绘图 | `draw_point` | x, y | 直接绘制点 |
| 程序绘图 | `draw_line` | x1, y1, x2, y2 | 直接绘制线 |
| 程序绘图 | `draw_polyline` | points | 绘制多段线 |
| 程序绘图 | `draw_polygon` | points | 绘制多边形 |
| 程序绘图 | `draw_circle` | cx, cy, radius | 直接绘制圆 |
| 程序绘图 | `draw_arc` | cx, cy, radius, start_angle, sweep_angle | 直接绘制圆弧 |
| 删除 | `delete` | - | 删除选中对象 |

**工具调用格式**：
- 单工具：`{"tool": "tool_name", "parameters": {...}, "message": "说明"}`
- 多工具：`[{"tool": "...", ...}, {"tool": "...", ...}]`

**JSON 解析流程**：
1. 从 AI 响应中提取 JSON（支持 markdown 代码块）
2. 检查是否为数组（多工具调用）
3. 遍历执行每个工具调用
4. 绘制工具执行完后自动调用 fit_all

---

## 七、第三方库

**SARibbon 2.9.0**：
- Office 风格 Ribbon 界面
- 支持多种主题（Office 2013/2016/2021 等）
- 包含完整的 Ribbon 组件体系
- 单头文件集成方式

**OpenSSL 1.1.1w**：
- HTTPS 加密通信
- Qt 5.14.2 必须使用 1.1.x 版本
- 提供 libcrypto 和 libssl 库

---

## 八、状态栏

**左侧**：状态消息（默认 Ready）
**右侧**：坐标显示（12pt 粗体，250px 宽）

坐标通过 ray-plane intersection 将屏幕坐标映射到 Z=0 平面，实时更新。

---

## 九、编译与运行

**环境要求**：
- Visual Studio 2019 (v142)
- Qt 5.14.2 (msvc2017_64)
- OpenCASCADE 7.x
- OpenSSL 1.1.1

**环境变量**：
- `QTDIR`：Qt 安装目录
- `CASROOT`：OpenCASCADE 安装目录
- `OPENSSL_ROOT_DIR`：OpenSSL 安装目录

**编译步骤**：
1. 打开 `LzzCad.sln`
2. 设置平台为 x64
3. 配置为 Debug 或 Release
4. 编译项目

---

## 十、版本历史

| 版本 | 主要更新 |
|------|----------|
| v1.0 | 基础框架，OCC 视图，文件读写 |
| v1.1 | SARibbon 界面 + Dock 面板 |
| v1.2 | AI 对话，DeepSeek API |
| v1.3 | 草图绘制（点/线/圆/弧） |
| v1.4 | Undo/Redo 系统，API 优化 |

---

## 十一、已知问题

1. `Command.h` 单文件需拆分
2. popup() 为空，多个按钮未连接槽函数
3. 模型树/属性面板为静态数据
4. API Key 明文存储存在安全风险
