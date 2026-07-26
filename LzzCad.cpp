#include "LzzCad.h"
#include "view.h"
#include <QAction>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QIcon>
#include <QPixmap>
#include <QFileDialog>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QInputDialog>
#include<qheaderview.h>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax2.hxx>
#include <gp_Pln.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <GeomAPI_IntCS.hxx>
#include <Graphic3d_Camera.hxx>
#include <Quantity_Color.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <Standard_Failure.hxx>
#include <Standard_ErrorHandler.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>

LzzCad::LzzCad(QWidget* parent)
    : SARibbonMainWindow(parent)
    , m_networkManager(nullptr)
    , m_isProcessing(false)
    , m_commandManager(new CommandManager(50))
{
    ui.setupUi(this);

    setWindowTitle("LzzCad - CAD Platform");
    resize(1600, 1200);

    createRibbon();
    setupWindowIcon();

    setRibbonTheme(SARibbonTheme::RibbonThemeOffice2021Blue);

    SARibbonSystemButtonBar* buttonBar = windowButtonBar();
    if (buttonBar) {
        buttonBar->setMinimumWidth(180);
    }

    createCentralWidget();
    createDockWidgets();

    statusBar()->setStyleSheet("QStatusBar { background-color: #ffffff; color: #333333; border-top: 1px solid #e0e0e0; min-height: 28px; }");
    statusBar()->showMessage("Ready");
    
    m_positionLabel = new QLabel("X: 0.000   Y: 0.000   Z: 0.000");
    m_positionLabel->setMinimumWidth(250);
    QFont font = m_positionLabel->font();
    font.setPointSize(12);
    font.setBold(true);
    m_positionLabel->setFont(font);
    statusBar()->addPermanentWidget(m_positionLabel);
    
    connect(m_occView, &OccView::mouseMoved, this, &LzzCad::updatePositionLabel);

    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &LzzCad::onApiResponse);
    
    m_apiUrl = "https://api.deepseek.com/v1/chat/completions";
    m_apiKey = "";
    
    addLogMessage("[INFO] AI assistant initialized");
    addLogMessage("[INFO] Enter API key in Help -> Settings to enable AI features");
}

LzzCad::~LzzCad()
{
    delete m_commandManager;
}

QIcon LzzCad::loadIcon(const QString& iconName)
{
    QString iconPath = QString("E:/MyGithub/LzzCad/image/ribbon/icon_%1.png").arg(iconName);
    QPixmap pixmap(iconPath);
    if (!pixmap.isNull()) {
        QPixmap scaledPixmap = pixmap.scaledToWidth(24, Qt::SmoothTransformation);
        return QIcon(scaledPixmap);
    } else {
        qWarning() << "Failed to load icon:" << iconPath;
        return QIcon();
    }
}

void LzzCad::setupWindowIcon()
{
    QString iconPath = "E:/MyGithub/LzzCad/image/logo/lzz.png";
    QPixmap pixmap(iconPath);
    if (!pixmap.isNull()) {
        setWindowIcon(QIcon(pixmap));
        
        SARibbonBar* ribbon = ribbonBar();
        if (ribbon) {
            SARibbonTitleIconWidget* titleIcon = ribbon->titleIconWidget();
            if (titleIcon) {
                titleIcon->setIcon(QIcon(pixmap));
            }
            ribbon->setTitleIconVisible(true);
        }
    } else {
        qWarning() << "Failed to load logo from:" << iconPath;
    }
}

void LzzCad::createCentralWidget()
{
    m_geometryModel = new GeometryModel();
    
    m_occViewModel = new OccViewModel(m_geometryModel);
    
    m_occView = new OccView(this);
    m_occView->setViewModel(m_occViewModel);
    m_occView->setCommandManager(m_commandManager);
    
    connect(m_occViewModel, &OccViewModel::requestAddShape, m_occView, &OccView::displayShape);
    connect(m_occViewModel, &OccViewModel::requestFitAll, m_occView, &OccView::fitAll);
    connect(m_occViewModel, &OccViewModel::requestRotate, m_occView, &OccView::rotate);
    connect(m_occViewModel, &OccViewModel::requestPan, m_occView, &OccView::pan);
    connect(m_occViewModel, &OccViewModel::requestZoom, m_occView, &OccView::zoom);
    connect(m_occViewModel, &OccViewModel::requestViewTop, m_occView, &OccView::top);
    connect(m_occViewModel, &OccViewModel::requestViewBottom, m_occView, &OccView::bottom);
    connect(m_occViewModel, &OccViewModel::requestViewLeft, m_occView, &OccView::left);
    connect(m_occViewModel, &OccViewModel::requestViewRight, m_occView, &OccView::right);
    connect(m_occViewModel, &OccViewModel::requestViewFront, m_occView, &OccView::front);
    connect(m_occViewModel, &OccViewModel::requestViewBack, m_occView, &OccView::back);
    
    connect(m_occViewModel, &OccViewModel::shapeAddedToModel, this, &LzzCad::addModelTreeItem);
    
    connect(m_occView, &OccView::shapeCreated, [this](const Handle(AIS_Shape)& shape, ShapeType type) {
        if (m_occViewModel) {
            switch (type) {
            case ShapeType::Point: m_occViewModel->addPoint(shape); break;
            case ShapeType::Line: m_occViewModel->addLine(shape); break;
            case ShapeType::Circle: m_occViewModel->addCircle(shape); break;
            case ShapeType::Arc: m_occViewModel->addArc(shape); break;
            case ShapeType::Extrude: m_occViewModel->addShapeWithType(shape, type); break;
            case ShapeType::Revolve: m_occViewModel->addShapeWithType(shape, type); break;
            case ShapeType::Sweep: m_occViewModel->addShapeWithType(shape, type); break;
            default: m_occViewModel->addShapeWithType(shape, type); break;
            }
        }
    });

    connect(m_occView, &OccView::selectionChanged, [this]() {
        QList<Handle(AIS_InteractiveObject)> selectedObjects = m_occView->getSelectedObjects();
        if (!selectedObjects.isEmpty() && m_propertyPanel && m_geometryModel) {
            Handle(AIS_Shape) selectedShape = Handle(AIS_Shape)::DownCast(selectedObjects.first());
            if (!selectedShape.IsNull()) {
                const auto& shapes = m_geometryModel->getShapes();
                const auto& names = m_geometryModel->getShapeNames();
                const auto& types = m_geometryModel->getShapeTypes();
                for (size_t i = 0; i < shapes.size(); ++i) {
                    if (shapes[i] == selectedShape) {
                        m_propertyPanel->updateSelection(shapes[i], names[i], types[i]);
                        break;
                    }
                }
            }
        }
    });
    
    setCentralWidget(m_occView);
}

void LzzCad::createDockWidgets()
{
    createModelTreeDock();
    createPropertyDock();
    createLogDock();
}

void LzzCad::createModelTreeDock()
{
    QDockWidget* modelDock = new QDockWidget("Model Tree", this);
    modelDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    modelDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
    // 默认白色 Dock Widget 样式
    modelDock->setStyleSheet(
        "QDockWidget { "
        "  background-color: #ffffff; "
        "  color: #333333; "
        "  border: 1px solid #e0e0e0; "
        "} "
        "QDockWidget::title { "
        "  background-color: #f5f5f5; "
        "  color: #333333; "
        "  padding: 4px; "
        "  border-bottom: 1px solid #e0e0e0; "
        "}"
    );

    m_modelTree = new QTreeWidget();
    m_modelTree->setColumnCount(2);
    QStringList headers;
    headers << "Name" << "";
    m_modelTree->setHeaderLabels(headers);
    m_modelTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_modelTree->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_modelTree->header()->resizeSection(1, 24);
    // 默认白色 TreeWidget 样式
    m_modelTree->setStyleSheet(
        "QTreeWidget { "
        "  background-color: #ffffff; "
        "  color: #333333; "
        "  border: 1px solid #e0e0e0; "
        "} "
        "QTreeWidget::item { "
        "  padding: 4px; "
        "} "
        "QTreeWidget::item:selected { "
        "  background-color: #cce5ff; "
        "  color: #0066cc; "
        "} "
        "QHeaderView { "
        "  background-color: #f5f5f5; "
        "}"
    );

    QTreeWidgetItem* rootItem = new QTreeWidgetItem(m_modelTree);
    rootItem->setText(0, "Assembly");
    rootItem->setExpanded(true);
    m_modelTree->addTopLevelItem(rootItem);

    connect(m_modelTree, &QTreeWidget::itemClicked, this, &LzzCad::onModelTreeItemClicked);
    
    m_showIcon = QIcon("E:/MyGithub/LzzCad/image/modeltree/show.png");
    m_hideIcon = QIcon("E:/MyGithub/LzzCad/image/modeltree/hide.png");

    modelDock->setWidget(m_modelTree);
    addDockWidget(Qt::LeftDockWidgetArea, modelDock);
}

void LzzCad::createPropertyDock()
{
    QDockWidget* propertyDock = new QDockWidget("Properties", this);
    propertyDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    propertyDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
    // 默认白色 Dock Widget 样式
    propertyDock->setStyleSheet(
        "QDockWidget { "
        "  background-color: #ffffff; "
        "  color: #333333; "
        "  border: 1px solid #e0e0e0; "
        "} "
        "QDockWidget::title { "
        "  background-color: #f5f5f5; "
        "  color: #333333; "
        "  padding: 4px; "
        "  border-bottom: 1px solid #e0e0e0; "
        "}"
    );

    m_propertyPanel = new PropertyPanel();
    propertyDock->setWidget(m_propertyPanel);
    addDockWidget(Qt::RightDockWidgetArea, propertyDock);
    
    connect(m_propertyPanel, &PropertyPanel::applyTransform, this, [this](double x, double y, double z, double rx, double ry, double rz) {
        QTreeWidgetItem* selectedItem = m_modelTree->currentItem();
        if (selectedItem && selectedItem->parent()) {
            QString itemName = selectedItem->text(0);
            const auto& shapes = m_geometryModel->getShapes();
            const auto& names = m_geometryModel->getShapeNames();
            const auto& types = m_geometryModel->getShapeTypes();
            for (size_t i = 0; i < names.size(); ++i) {
                if (names[i] == itemName) {
                    m_occView->transformShape(shapes[i], x, y, z, rx, ry, rz);
                    if (m_propertyPanel) {
                        m_propertyPanel->updateSelection(shapes[i], names[i], types[i]);
                    }
                    break;
                }
            }
        }
    });

    connect(m_propertyPanel, &PropertyPanel::colorChanged, this, [this](const Handle(AIS_Shape)& shape, const Quantity_Color& color) {
        shape->SetColor(color);
        m_occView->updateViewer();
    });
}

void LzzCad::createLogDock()
{
    QDockWidget* logDock = new QDockWidget("Log & AI", this);
    logDock->setAllowedAreas(Qt::BottomDockWidgetArea);
    logDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
    logDock->setMaximumHeight(200);
    logDock->setStyleSheet(
        "QDockWidget { "
        "  background-color: #ffffff; "
        "  color: #333333; "
        "  border: 1px solid #e0e0e0; "
        "} "
        "QDockWidget::title { "
        "  background-color: #f5f5f5; "
        "  color: #333333; "
        "  padding: 4px; "
        "  border-bottom: 1px solid #e0e0e0; "
        "}"
    );

    QWidget* containerWidget = new QWidget();
    QHBoxLayout* mainLayout = new QHBoxLayout(containerWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(2);

    // Left: Log panel (50%)
    QWidget* logWidget = new QWidget();
    QVBoxLayout* logLayout = new QVBoxLayout(logWidget);
    logLayout->setContentsMargins(4, 4, 4, 4);
    
    QLabel* logLabel = new QLabel("Log");
    logLabel->setStyleSheet("font-weight: bold; color: #333333;");
    logLayout->addWidget(logLabel);
    
    m_logEdit = new QTextEdit();
    m_logEdit->setReadOnly(true);
    m_logEdit->setStyleSheet("QTextEdit { background-color: #f8f9fa; color: #333333; border: 1px solid #e0e0e0; font-family: Consolas; font-size: 11px; }");
    m_logEdit->append("[INFO] LzzCad started");
    m_logEdit->append("[INFO] Interface initialized");
    logLayout->addWidget(m_logEdit);
    mainLayout->addWidget(logWidget, 1);

    // Vertical separator
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setStyleSheet("background-color: #e0e0e0;");
    mainLayout->addWidget(separator);

    // Right: AI Chat panel (50%)
    QWidget* chatWidget = new QWidget();
    QVBoxLayout* chatLayout = new QVBoxLayout(chatWidget);
    chatLayout->setContentsMargins(4, 4, 4, 4);
    
    QLabel* chatLabel = new QLabel("AI Assistant");
    chatLabel->setStyleSheet("font-weight: bold; color: #333333;");
    chatLayout->addWidget(chatLabel);
    
    m_chatHistory = new QTextEdit();
    m_chatHistory->setReadOnly(true);
    m_chatHistory->setStyleSheet("QTextEdit { background-color: #ffffff; color: #333333; border: 1px solid #e0e0e0; font-size: 12px; }");
    m_chatHistory->append("<b>AI:</b> Hello! I'm your CAD assistant. How can I help you today?");
    chatLayout->addWidget(m_chatHistory);
    
    QHBoxLayout* inputLayout = new QHBoxLayout();
    m_chatInput = new QLineEdit();
    m_chatInput->setPlaceholderText("Type a message...");
    m_chatInput->setStyleSheet("QLineEdit { border: 1px solid #e0e0e0; padding: 6px; font-size: 14px; }");
    inputLayout->addWidget(m_chatInput);
    
    m_sendButton = new QPushButton("Send");
    m_sendButton->setStyleSheet(
        "QPushButton { "
        "  background-color: #0078d7; "
        "  color: white; "
        "  border: none; "
        "  padding: 4px 12px; "
        "  border-radius: 4px; "
        "} "
        "QPushButton:hover { background-color: #005a9e; } "
        "QPushButton:disabled { background-color: #cccccc; }"
    );
    inputLayout->addWidget(m_sendButton);
    chatLayout->addLayout(inputLayout);
    mainLayout->addWidget(chatWidget, 1);

    connect(m_sendButton, &QPushButton::clicked, this, &LzzCad::onSendMessage);
    connect(m_chatInput, &QLineEdit::returnPressed, this, &LzzCad::onSendMessage);

    logDock->setWidget(containerWidget);
    addDockWidget(Qt::BottomDockWidgetArea, logDock);
}

void LzzCad::createRibbon()
{
    SARibbonBar* ribbon = ribbonBar();

    if (!ribbon) {
        ribbon = new SARibbonBar(this);
        setRibbonBar(ribbon);
    }

    ribbon->setRibbonStyle(SARibbonBar::RibbonStyleCompactThreeRow);

    // Create ribbon categories in order
    createFileCategory(ribbon);
    createSketchCategory(ribbon);
    createModelCategory(ribbon);
    createViewCategory(ribbon);
    createToolsCategory(ribbon);
    createCAMCategory(ribbon);
    createHelpCategory(ribbon);
}

// File category: basic file operations
void LzzCad::createFileCategory(SARibbonBar* ribbon)
{
    SARibbonCategory* fileCategory = ribbon->addCategoryPage("File");
    if (!fileCategory) return;

    // File operations panel
    SARibbonPanel* filePanel = fileCategory->addPanel("File");
    
    QAction* newAction = new QAction(loadIcon("new"), "New", this);
    filePanel->addLargeAction(newAction);
    
    QAction* openAction = new QAction(loadIcon("open"), "Open", this);
    connect(openAction, &QAction::triggered, this, &LzzCad::onFileOpen);
    filePanel->addLargeAction(openAction);
    
    QAction* saveAction = new QAction(loadIcon("save"), "Save", this);
    connect(saveAction, &QAction::triggered, this, &LzzCad::onFileSave);
    filePanel->addLargeAction(saveAction);
    
    QAction* saveAsAction = new QAction(loadIcon("save_as"), "Save As", this);
    filePanel->addLargeAction(saveAsAction);

    // Edit operations panel
    SARibbonPanel* editPanel = fileCategory->addPanel("Edit");
    
    QAction* undoAction = new QAction(loadIcon("undo"), "Undo", this);
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, this, &LzzCad::onUndo);
    editPanel->addLargeAction(undoAction);
    addAction(undoAction);
    
    QAction* redoAction = new QAction(loadIcon("redo"), "Redo", this);
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered, this, &LzzCad::onRedo);
    editPanel->addLargeAction(redoAction);
    addAction(redoAction);

    // Window operations panel
    SARibbonPanel* windowPanel = fileCategory->addPanel("Window");
    
    QAction* closeAction = new QAction(loadIcon("close"), "Close", this);
    windowPanel->addLargeAction(closeAction);
    
    QAction* exitAction = new QAction(loadIcon("exit"), "Exit", this);
    windowPanel->addLargeAction(exitAction);
    connect(exitAction, &QAction::triggered, this, &LzzCad::close);
}

// Sketch category: 2D sketch drawing tools
void LzzCad::createSketchCategory(SARibbonBar* ribbon)
{
    SARibbonCategory* sketchCategory = ribbon->addCategoryPage("Sketch");
    if (!sketchCategory) return;

    // Basic drawing panel
    SARibbonPanel* basicPanel = sketchCategory->addPanel("Basic");
    
    QAction* pointAction = new QAction(loadIcon("point"), "Point", this);
    connect(pointAction, &QAction::triggered, this, &LzzCad::onSketchPoint);
    basicPanel->addLargeAction(pointAction);
    
    QAction* lineAction = new QAction(loadIcon("line"), "Line", this);
    connect(lineAction, &QAction::triggered, this, &LzzCad::onSketchLine);
    basicPanel->addLargeAction(lineAction);
    
    QAction* circleAction = new QAction(loadIcon("circle"), "Circle", this);
    connect(circleAction, &QAction::triggered, this, &LzzCad::onSketchCircle);
    basicPanel->addLargeAction(circleAction);
    
    QAction* arcAction = new QAction(loadIcon("arc"), "Arc", this);
    connect(arcAction, &QAction::triggered, this, &LzzCad::onSketchArc);
    basicPanel->addLargeAction(arcAction);

    // Curve drawing panel
    SARibbonPanel* curvePanel = sketchCategory->addPanel("Curve");
    
    QAction* polylineAction = new QAction(loadIcon("polyline"), "Polyline", this);
    curvePanel->addLargeAction(polylineAction);
    connect(polylineAction, &QAction::triggered, this, &LzzCad::onSketchPolyline);
    
    QAction* splineAction = new QAction(loadIcon("spline"), "Spline", this);
    curvePanel->addLargeAction(splineAction);
    connect(splineAction, &QAction::triggered, this, &LzzCad::onSketchSpline);
    
    QAction* ellipseAction = new QAction(loadIcon("ellipse"), "Ellipse", this);
    curvePanel->addLargeAction(ellipseAction);
    connect(ellipseAction, &QAction::triggered, this, &LzzCad::onSketchEllipse);
    
    QAction* rectangleAction = new QAction(loadIcon("rectangle"), "Rectangle", this);
    curvePanel->addLargeAction(rectangleAction);
    connect(rectangleAction, &QAction::triggered, this, &LzzCad::onSketchRectangle);
}

// Model category: 3D modeling operations
void LzzCad::createModelCategory(SARibbonBar* ribbon)
{
    SARibbonCategory* modelCategory = ribbon->addCategoryPage("Model");
    if (!modelCategory) return;

    // Primitives panel
    SARibbonPanel* primitivesPanel = modelCategory->addPanel("Primitives");
    
    QAction* boxAction = new QAction(loadIcon("box"), "Box", this);
    primitivesPanel->addLargeAction(boxAction);
    connect(boxAction, &QAction::triggered, this, &LzzCad::onCreateBox);
    
    QAction* cylinderAction = new QAction(loadIcon("cylinder"), "Cylinder", this);
    primitivesPanel->addLargeAction(cylinderAction);
    connect(cylinderAction, &QAction::triggered, this, &LzzCad::onCreateCylinder);
    
    QAction* sphereAction = new QAction(loadIcon("sphere"), "Sphere", this);
    primitivesPanel->addLargeAction(sphereAction);
    connect(sphereAction, &QAction::triggered, this, &LzzCad::onCreateSphere);
    
    QAction* coneAction = new QAction(loadIcon("cone"), "Cone", this);
    primitivesPanel->addLargeAction(coneAction);
    connect(coneAction, &QAction::triggered, this, &LzzCad::onCreateCone);

    // Features panel (extrude, revolve, etc.)
    SARibbonPanel* featuresPanel = modelCategory->addPanel("Features");
    
    QAction* extrudeAction = new QAction(loadIcon("extrude"), "Extrude", this);
    featuresPanel->addLargeAction(extrudeAction);
    connect(extrudeAction, &QAction::triggered, this, &LzzCad::onCreateExtrude);
    
    QAction* revolveAction = new QAction(loadIcon("revolve"), "Revolve", this);
    featuresPanel->addLargeAction(revolveAction);
    connect(revolveAction, &QAction::triggered, this, &LzzCad::onCreateRevolve);
    
    QAction* sweepAction = new QAction(loadIcon("sweep"), "Sweep", this);
    featuresPanel->addLargeAction(sweepAction);
    connect(sweepAction, &QAction::triggered, this, &LzzCad::onCreateSweep);
    
    QAction* loftAction = new QAction(loadIcon("loft"), "Loft", this);
    featuresPanel->addLargeAction(loftAction);

    // Modify panel (chamfer, fillet, etc.)
    SARibbonPanel* modifyPanel = modelCategory->addPanel("Modify");
    
    QAction* chamferAction = new QAction(loadIcon("chamfer"), "Chamfer", this);
    modifyPanel->addLargeAction(chamferAction);
    
    QAction* filletAction = new QAction(loadIcon("fillet"), "Fillet", this);
    modifyPanel->addLargeAction(filletAction);
    
    QAction* draftAction = new QAction(loadIcon("draft"), "Draft", this);
    modifyPanel->addLargeAction(draftAction);
    
    QAction* shellAction = new QAction(loadIcon("shell"), "Shell", this);
    modifyPanel->addLargeAction(shellAction);

    // Boolean operations panel
    SARibbonPanel* booleanPanel = modelCategory->addPanel("Boolean");
    
    QAction* fuseAction = new QAction(loadIcon("union"), "Union", this);
    booleanPanel->addLargeAction(fuseAction);
    connect(fuseAction, &QAction::triggered, this, &LzzCad::onBooleanUnion);
    
    QAction* cutAction = new QAction(loadIcon("cut"), "Cut", this);
    booleanPanel->addLargeAction(cutAction);
    connect(cutAction, &QAction::triggered, this, &LzzCad::onBooleanCut);
    
    QAction* intersectAction = new QAction(loadIcon("intersect"), "Intersect", this);
    booleanPanel->addLargeAction(intersectAction);
    connect(intersectAction, &QAction::triggered, this, &LzzCad::onBooleanIntersect);
}

// View category: view manipulation and display
void LzzCad::createViewCategory(SARibbonBar* ribbon)
{
    SARibbonCategory* viewCategory = ribbon->addCategoryPage("View");
    if (!viewCategory) return;

    // Standard views panel
    SARibbonPanel* standardViewsPanel = viewCategory->addPanel("Standard Views");
    
    QAction* topViewAction = new QAction(loadIcon("top"), "Top", this);
    connect(topViewAction, &QAction::triggered, this, &LzzCad::onViewTop);
    standardViewsPanel->addLargeAction(topViewAction);
    
    QAction* bottomViewAction = new QAction(loadIcon("bottom"), "Bottom", this);
    connect(bottomViewAction, &QAction::triggered, this, &LzzCad::onViewBottom);
    standardViewsPanel->addLargeAction(bottomViewAction);
    
    QAction* frontViewAction = new QAction(loadIcon("front"), "Front", this);
    connect(frontViewAction, &QAction::triggered, this, &LzzCad::onViewFront);
    standardViewsPanel->addLargeAction(frontViewAction);
    
    QAction* backViewAction = new QAction(loadIcon("back"), "Back", this);
    connect(backViewAction, &QAction::triggered, this, &LzzCad::onViewBack);
    standardViewsPanel->addLargeAction(backViewAction);
    
    QAction* leftViewAction = new QAction(loadIcon("left"), "Left", this);
    connect(leftViewAction, &QAction::triggered, this, &LzzCad::onViewLeft);
    standardViewsPanel->addLargeAction(leftViewAction);
    
    QAction* rightViewAction = new QAction(loadIcon("right"), "Right", this);
    connect(rightViewAction, &QAction::triggered, this, &LzzCad::onViewRight);
    standardViewsPanel->addLargeAction(rightViewAction);

    // Display modes panel
    SARibbonPanel* displayPanel = viewCategory->addPanel("Display");
    
    QAction* wireframeAction = new QAction(loadIcon("wireframe"), "Wireframe", this);
    connect(wireframeAction, &QAction::triggered, this, &LzzCad::onViewWireframe);
    displayPanel->addLargeAction(wireframeAction);
    
    QAction* shadedAction = new QAction(loadIcon("shaded"), "Shaded", this);
    connect(shadedAction, &QAction::triggered, this, &LzzCad::onViewShaded);
    displayPanel->addLargeAction(shadedAction);
    
    QAction* shadedEdgesAction = new QAction(loadIcon("shaded_edges"), "Shaded Edges", this);
    displayPanel->addLargeAction(shadedEdgesAction);

    // Camera controls panel
    SARibbonPanel* cameraPanel = viewCategory->addPanel("Camera");
    
    QAction* fitAllAction = new QAction(loadIcon("fit_all"), "Fit All", this);
    connect(fitAllAction, &QAction::triggered, this, &LzzCad::onViewFitAll);
    cameraPanel->addLargeAction(fitAllAction);
    
    QAction* zoomInAction = new QAction(loadIcon("zoom_in"), "Zoom In", this);
    cameraPanel->addLargeAction(zoomInAction);
    
    QAction* zoomOutAction = new QAction(loadIcon("zoom_out"), "Zoom Out", this);
    cameraPanel->addLargeAction(zoomOutAction);
    
    QAction* panAction = new QAction(loadIcon("pan"), "Pan", this);
    connect(panAction, &QAction::triggered, this, &LzzCad::onViewPan);
    cameraPanel->addLargeAction(panAction);
    
    QAction* rotateAction = new QAction(this);
    rotateAction->setText("Rotate");
    QString rotateIconPath = "E:/MyGithub/LzzCad/image/ribbon/icon_rotate.png";
    QPixmap rotatePixmap(rotateIconPath);
    if (!rotatePixmap.isNull()) {
        QPixmap scaledRotatePixmap = rotatePixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        rotateAction->setIcon(QIcon(scaledRotatePixmap));
    } else {
        rotateAction->setIcon(loadIcon("rotate"));
    }
    connect(rotateAction, &QAction::triggered, this, &LzzCad::onViewRotate);
    cameraPanel->addLargeAction(rotateAction);
}

// Tools category: measurement and analysis tools
void LzzCad::createToolsCategory(SARibbonBar* ribbon)
{
    SARibbonCategory* toolsCategory = ribbon->addCategoryPage("Tools");
    if (!toolsCategory) return;

    // Edit panel
    SARibbonPanel* editPanel = toolsCategory->addPanel("Edit");
    
    QAction* deleteAction = new QAction(loadIcon("delete"), "Delete", this);
    deleteAction->setShortcut(QKeySequence::Delete);
    connect(deleteAction, &QAction::triggered, this, &LzzCad::onDeleteSelected);
    editPanel->addLargeAction(deleteAction);
    addAction(deleteAction);

    // Measure panel
    SARibbonPanel* measurePanel = toolsCategory->addPanel("Measure");
    
    QAction* distanceAction = new QAction(loadIcon("distance"), "Distance", this);
    measurePanel->addLargeAction(distanceAction);
    
    QAction* angleAction = new QAction(loadIcon("angle"), "Angle", this);
    measurePanel->addLargeAction(angleAction);
    
    QAction* areaAction = new QAction(loadIcon("area"), "Area", this);
    measurePanel->addLargeAction(areaAction);
    
    QAction* volumeAction = new QAction(loadIcon("volume"), "Volume", this);
    measurePanel->addLargeAction(volumeAction);

    // Analysis panel
    SARibbonPanel* analysisPanel = toolsCategory->addPanel("Analysis");
    
    QAction* checkGeometryAction = new QAction(loadIcon("check_geometry"), "Check Geometry", this);
    analysisPanel->addLargeAction(checkGeometryAction);
    
    QAction* massPropsAction = new QAction(loadIcon("mass_properties"), "Mass Properties", this);
    analysisPanel->addLargeAction(massPropsAction);

    // Point cloud panel
    SARibbonPanel* pointCloudPanel = toolsCategory->addPanel("Point Cloud");
    
    QAction* importPointCloudAction = new QAction(loadIcon("import_pointcloud"), "Import", this);
    pointCloudPanel->addLargeAction(importPointCloudAction);
    
    QAction* reconstructAction = new QAction(loadIcon("reconstruct"), "Reconstruct", this);
    pointCloudPanel->addLargeAction(reconstructAction);
    
    QAction* reverseEngineerAction = new QAction(loadIcon("reverse_engineer"), "Reverse Eng.", this);
    pointCloudPanel->addLargeAction(reverseEngineerAction);
    
    QAction* meshAction = new QAction(loadIcon("mesh"), "Mesh", this);
    pointCloudPanel->addLargeAction(meshAction);

    // Screenshot panel
    SARibbonPanel* screenshotPanel = toolsCategory->addPanel("Screenshot");
    
    QAction* captureScreenAction = new QAction(loadIcon("capture_screen"), "Capture Screen", this);
    screenshotPanel->addLargeAction(captureScreenAction);
    
    QAction* captureSelectionAction = new QAction(loadIcon("capture_selection"), "Capture Selection", this);
    screenshotPanel->addLargeAction(captureSelectionAction);
    
    QAction* captureWindowAction = new QAction(loadIcon("capture_window"), "Capture Window", this);
    screenshotPanel->addLargeAction(captureWindowAction);
}

// CAM category: CAM operations
void LzzCad::createCAMCategory(SARibbonBar* ribbon)
{
    SARibbonCategory* camCategory = ribbon->addCategoryPage("CAM");
    if (!camCategory) return;

    // Projection panel
    SARibbonPanel* projectionPanel = camCategory->addPanel("Projection");
    
    QAction* projectToSurfaceAction = new QAction(loadIcon("project_to_surface"), "To Surface", this);
    projectionPanel->addLargeAction(projectToSurfaceAction);
    
    QAction* projectToPlaneAction = new QAction(loadIcon("project_to_plane"), "To Plane", this);
    projectionPanel->addLargeAction(projectToPlaneAction);
    
    QAction* projectAlongDirAction = new QAction(loadIcon("project_along_dir"), "Along Dir", this);
    projectionPanel->addLargeAction(projectAlongDirAction);

    // Toolpath panel
    SARibbonPanel* toolpathPanel = camCategory->addPanel("Toolpath");
    
    QAction* pocketAction = new QAction(loadIcon("pocket"), "Pocket", this);
    toolpathPanel->addLargeAction(pocketAction);
    
    QAction* contourAction = new QAction(loadIcon("contour"), "Contour", this);
    toolpathPanel->addLargeAction(contourAction);
    
    QAction* drillingAction = new QAction(loadIcon("drilling"), "Drilling", this);
    toolpathPanel->addLargeAction(drillingAction);
    
    QAction* engravingAction = new QAction(loadIcon("engraving"), "Engraving", this);
    toolpathPanel->addLargeAction(engravingAction);

    // Output panel
    SARibbonPanel* outputPanel = camCategory->addPanel("Output");
    
    QAction* fillAction = new QAction(loadIcon("fill"), "Fill", this);
    outputPanel->addLargeAction(fillAction);
    
    QAction* generateGCodeAction = new QAction(loadIcon("generate_gcode"), "Generate G-Code", this);
    outputPanel->addLargeAction(generateGCodeAction);
    
    QAction* simulateAction = new QAction(loadIcon("simulate"), "Simulate", this);
    outputPanel->addLargeAction(simulateAction);
}

// Help category: help and support
void LzzCad::createHelpCategory(SARibbonBar* ribbon)
{
    SARibbonCategory* helpCategory = ribbon->addCategoryPage("Help");
    if (!helpCategory) return;

    SARibbonPanel* helpPanel = helpCategory->addPanel("Help");
    
    QAction* guideAction = new QAction(loadIcon("guide"), "Guide", this);
    connect(guideAction, &QAction::triggered, this, [this]() {
        addLogMessage("[GUIDE] === LzzCad Quick Guide ===");
        addLogMessage("[GUIDE] Basic Operations:");
        addLogMessage("[GUIDE]   - Left click: Select objects");
        addLogMessage("[GUIDE]   - Middle click/drag: Rotate view");
        addLogMessage("[GUIDE]   - Right click: Cancel/Context menu");
        addLogMessage("[GUIDE]   - Scroll wheel: Zoom in/out");
        addLogMessage("[GUIDE]");
        addLogMessage("[GUIDE] Sketch Mode (Draw Tab):");
        addLogMessage("[GUIDE]   - Point: Click to place points");
        addLogMessage("[GUIDE]   - Line: Click start point, click end point");
        addLogMessage("[GUIDE]   - Circle: Click center, drag to set radius");
        addLogMessage("[GUIDE]   - Arc: Click start, click mid, click end");
        addLogMessage("[GUIDE]   - ESC: Exit sketch mode");
        addLogMessage("[GUIDE]");
        addLogMessage("[GUIDE] Primitive Creation (Model Tab):");
        addLogMessage("[GUIDE]   - Box: Click 2 corners for base, enter height");
        addLogMessage("[GUIDE]   - Sphere: Click center, drag to set radius");
        addLogMessage("[GUIDE]   - Cylinder: Click center, drag radius, enter height");
        addLogMessage("[GUIDE]   - Cone: Click center, drag radius, enter height");
        addLogMessage("[GUIDE]");
        addLogMessage("[GUIDE] Feature Operations (Model Tab):");
        addLogMessage("[GUIDE]   - Extrude: Select sketch, set height");
        addLogMessage("[GUIDE]   - Revolve: Select sketch, choose axis and angle");
        addLogMessage("[GUIDE]   - Sweep: Select profile then select path");
        addLogMessage("[GUIDE]");
        addLogMessage("[GUIDE] Property Panel:");
        addLogMessage("[GUIDE]   - Modify object properties (name, color, transform)");
        addLogMessage("[GUIDE]   - Change color via Color button");
        addLogMessage("[GUIDE]");
        addLogMessage("[GUIDE] File Operations:");
        addLogMessage("[GUIDE]   - Open: Load STEP/BREP/IGES files");
        addLogMessage("[GUIDE]   - Save: Save as STEP/BREP/IGES");
        addLogMessage("[GUIDE]   - Exit: Close the application");
        addLogMessage("[GUIDE]");
        addLogMessage("[GUIDE] Full documentation coming soon!");
    });
    helpPanel->addLargeAction(guideAction);
    
    QAction* bugReportAction = new QAction(loadIcon("bug_report"), "Bug Report", this);
    connect(bugReportAction, &QAction::triggered, this, [this]() {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Bug Report");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setText(
            "<h3>How to Report a Bug</h3>"
            "<p>Thank you for helping improve LzzCad!</p>"
            "<p>Please report bugs on GitHub:</p>"
            "<p><a href=\"https://github.com/tableli/LzzCad/issues\">https://github.com/tableli/LzzCad/issues</a></p>"
            "<p><b>When reporting, please include:</b></p>"
            "<ul>"
            "<li>Steps to reproduce the bug</li>"
            "<li>Expected behavior</li>"
            "<li>Actual behavior</li>"
            "<li>Screenshots (if applicable)</li>"
            "<li>Your OS and Qt version</li>"
            "</ul>"
        );
        msgBox.exec();
        addLogMessage("[INFO] Bug report info displayed. Please submit issues on GitHub.");
    });
    helpPanel->addLargeAction(bugReportAction);
    
    QAction* aboutAction = new QAction(loadIcon("about"), "About", this);
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("About LzzCad");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setText(
            "<h2>LzzCad</h2>"
            "<p>Version: 1.0.0</p>"
            "<p>A lightweight CAD software for 3D modeling and design.</p>"
            "<p></p>"
            "<p><b>Features:</b></p>"
            "<ul>"
            "<li>2D Sketching (Point, Line, Circle, Arc)</li>"
            "<li>3D Primitive Modeling (Box, Sphere, Cylinder, Cone)</li>"
            "<li>Feature Operations (Extrude, Revolve, Sweep)</li>"
            "<li>STEP/BREP/IGES File Support</li>"
            "<li>AI-powered Modeling Assistance</li>"
            "<li>Property Panel with Color Editing</li>"
            "</ul>"
            "<p></p>"
            "<p><b>Contact:</b></p>"
            "<p>Email: <a href=\"mailto:2521403134@qq.com\">2521403134@qq.com</a></p>"
            "<p>GitHub: <a href=\"https://github.com/tableli/LzzCad\">https://github.com/tableli/LzzCad</a></p>"
            "<p></p>"
            "<p><b>Copyright:</b></p>"
            "<p>&copy; 2026 tableli. All rights reserved.</p>"
            "<p>Built with Qt 5.14 and OpenCASCADE.</p>"
        );
        msgBox.exec();
        addLogMessage("[INFO] About dialog displayed");
    });
    helpPanel->addLargeAction(aboutAction);

    SARibbonPanel* settingsPanel = helpCategory->addPanel("Settings");
    
    QAction* apiSettingsAction = new QAction(this);
    apiSettingsAction->setText("API Settings");
    QString aiIconPath = "E:/MyGithub/LzzCad/image/ribbon/icon_ai_api.png";
    QPixmap aiPixmap(aiIconPath);
    if (!aiPixmap.isNull()) {
        QPixmap scaledAiPixmap = aiPixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        apiSettingsAction->setIcon(QIcon(scaledAiPixmap));
    } else {
        apiSettingsAction->setIcon(loadIcon("settings"));
    }
    connect(apiSettingsAction, &QAction::triggered, [this]() {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("API Settings");
        msgBox.setText("Choose API key input method:");
        QPushButton* manualBtn = msgBox.addButton("Manual Input", QMessageBox::ActionRole);
        QPushButton* fileBtn = msgBox.addButton("Load from File", QMessageBox::ActionRole);
        msgBox.addButton(QMessageBox::Cancel);
        
        msgBox.exec();
        
        if (msgBox.clickedButton() == manualBtn) {
            bool ok;
            QString apiKey = QInputDialog::getText(this, "API Settings", 
                "Enter your DeepSeek API key:", QLineEdit::Password, m_apiKey, &ok);
            if (ok && !apiKey.isEmpty()) {
                m_apiKey = apiKey;
                addLogMessage("[INFO] API key updated manually");
                m_chatHistory->append("<b>AI:</b> API key updated manually");
                testApiKey();
            }
        } else if (msgBox.clickedButton() == fileBtn) {
            QString filePath = QFileDialog::getOpenFileName(
                this,
                "Select API Key File",
                QString(),
                "Text Files (*.txt);;All Files (*.*)"
            );
            
            if (!filePath.isEmpty()) {
                QFile file(filePath);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QString apiKey = QString::fromUtf8(file.readAll()).trimmed();
                    file.close();
                    
                    if (!apiKey.isEmpty()) {
                        m_apiKey = apiKey;
                        addLogMessage(QString("[INFO] API key loaded from: %1").arg(filePath));
                        m_chatHistory->append(QString("<b>AI:</b> API key loaded from %1").arg(filePath));
                        testApiKey();
                    } else {
                        addLogMessage("[ERROR] API key file is empty");
                        m_chatHistory->append("<b>AI:</b> Error: API key file is empty");
                    }
                } else {
                    addLogMessage(QString("[ERROR] Failed to open file: %1").arg(filePath));
                    m_chatHistory->append(QString("<b>AI:</b> Error: Failed to open file"));
                }
            }
        }
    });
    settingsPanel->addLargeAction(apiSettingsAction);
}

// View slot implementations
void LzzCad::onViewFitAll()
{
    if (m_occViewModel) {
        m_occViewModel->fitAll();
    }
}

void LzzCad::onViewZoom()
{
    if (m_occViewModel) {
        m_occViewModel->zoom();
    }
}

void LzzCad::onViewPan()
{
    if (m_occView) {
        m_occView->pan();
    }
}

void LzzCad::onViewRotate()
{
    if (m_occView) {
        m_occView->rotate();
    }
}

void LzzCad::onViewTop()
{
    if (m_occViewModel) {
        m_occViewModel->viewTop();
    }
}

void LzzCad::onViewBottom()
{
    if (m_occViewModel) {
        m_occViewModel->viewBottom();
    }
}

void LzzCad::onViewLeft()
{
    if (m_occViewModel) {
        m_occViewModel->viewLeft();
    }
}

void LzzCad::onViewRight()
{
    if (m_occViewModel) {
        m_occViewModel->viewRight();
    }
}

void LzzCad::onViewFront()
{
    if (m_occViewModel) {
        m_occViewModel->viewFront();
    }
}

void LzzCad::onViewBack()
{
    if (m_occViewModel) {
        m_occViewModel->viewBack();
    }
}

void LzzCad::onViewShaded()
{
    if (m_occViewModel) {
        m_occViewModel->setShaded(true);
    }
}

void LzzCad::onViewWireframe()
{
    if (m_occViewModel) {
        m_occViewModel->setShaded(false);
    }
}

void LzzCad::onUndo()
{
    if (m_commandManager && m_commandManager->canUndo())
    {
        m_commandManager->undo();
        QString desc = m_commandManager->undoDescription();
        addLogMessage(QString("[Undo] %1").arg(desc.isEmpty() ? "Undone" : desc));
    }
    else
    {
        addLogMessage("[Undo] Nothing to undo");
    }
}

void LzzCad::onRedo()
{
    if (m_commandManager && m_commandManager->canRedo())
    {
        m_commandManager->redo();
        QString desc = m_commandManager->redoDescription();
        addLogMessage(QString("[Redo] %1").arg(desc.isEmpty() ? "Redone" : desc));
    }
    else
    {
        addLogMessage("[Redo] Nothing to redo");
    }
}

void LzzCad::onFileOpen()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open Model File",
        QString(),
        "STEP Files (*.step *.stp);;BREP Files (*.brep *.brp);;IGES Files (*.iges *.igs);;All Files (*.*)"
    );
    
    if (!fileName.isEmpty()) {
        if (m_occViewModel) {
            m_occViewModel->openFile(fileName);
        }
    }
}

void LzzCad::onFileSave()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save Model File",
        QString(),
        "STEP Files (*.step *.stp);;BREP Files (*.brep *.brp);;IGES Files (*.iges *.igs)"
    );
    
    if (!fileName.isEmpty()) {
        if (m_occViewModel) {
            m_occViewModel->saveFile(fileName);
        }
    }
}

void LzzCad::onClearAll()
{
    if (m_occViewModel) {
        m_occViewModel->clearAllShapes();
    }
    clearModelTree();
}

void LzzCad::onSketchPoint()
{
    // Switch to top view first
    if (m_occViewModel) {
        m_occViewModel->viewTop();
    }
    
    // Start sketch point mode
    if (m_occView) {
        m_occView->startSketchPointMode();
        addLogMessage("[Sketch] Point mode: Click once to place a point");
    }
}

void LzzCad::onSketchLine()
{
    // Switch to top view first
    if (m_occViewModel) {
        m_occViewModel->viewTop();
    }
    
    // Start sketch line mode
    if (m_occView) {
        m_occView->startSketchLineMode();
        addLogMessage("[Sketch] Line mode: Click to set start point, move mouse, click again to set end point");
    }
}

void LzzCad::onSketchCircle()
{
    // Switch to top view first
    if (m_occViewModel) {
        m_occViewModel->viewTop();
    }
    
    // Start sketch circle mode
    if (m_occView) {
        m_occView->startSketchCircleMode();
        addLogMessage("[Sketch] Circle mode: Click to set center, move mouse, click again to set radius");
    }
}

void LzzCad::onSketchArc()
{
    if (m_occViewModel) {
        m_occViewModel->viewTop();
    }
    if (m_occView) {
        m_occView->startSketchArcMode();
        addLogMessage("[Sketch] Arc mode: Click to set center, click for start point, click for end point");
    }
}


void LzzCad::onSketchPolyline()
{
    if (m_occViewModel) {
        m_occViewModel->viewTop();
    }
    if (m_occView) {
        m_occView->startSketchPolylineMode();
        addLogMessage("[Sketch] Polyline mode: Left-click to add points, right-click or Enter to finish");
    }
}

void LzzCad::onSketchSpline()
{
    if (m_occViewModel) {
        m_occViewModel->viewTop();
    }
    if (m_occView) {
        m_occView->startSketchSplineMode();
        addLogMessage("[Sketch] Spline mode: Left-click to add control points, right-click or Enter to finish");
    }
}

void LzzCad::onSketchEllipse()
{
    if (m_occViewModel) {
        m_occViewModel->viewTop();
    }
    if (m_occView) {
        m_occView->startSketchEllipseMode();
        addLogMessage("[Sketch] Ellipse mode: Click to set center, click again to set major radius");
    }
}

void LzzCad::onSketchRectangle()
{
    if (m_occViewModel) {
        m_occViewModel->viewTop();
    }
    if (m_occView) {
        m_occView->startSketchRectangleMode();
        addLogMessage("[Sketch] Rectangle mode: Click to set first corner, click again to set opposite corner");
    }
}
void LzzCad::onCreateBox()
{
    if (m_occViewModel) {
        m_occViewModel->viewTop();
    }
    if (m_occView) {
        m_occView->startPrimitiveBoxMode();
        addLogMessage("[Primitive] Box mode: Click to set first corner, click for second corner, click for height");
    }
}

void LzzCad::onCreateSphere()
{
    if (m_occViewModel) {
        m_occViewModel->viewTop();
    }
    if (m_occView) {
        m_occView->startPrimitiveSphereMode();
        addLogMessage("[Primitive] Sphere mode: Click to set center, click for radius");
    }
}

void LzzCad::onCreateCylinder()
{
    if (m_occViewModel) {
        m_occViewModel->viewTop();
    }
    if (m_occView) {
        m_occView->startPrimitiveCylinderMode();
        addLogMessage("[Primitive] Cylinder mode: Click to set center, click for radius, click for height");
    }
}

void LzzCad::onCreateCone()
{
    if (m_occViewModel) {
        m_occViewModel->viewTop();
    }
    if (m_occView) {
        m_occView->startPrimitiveConeMode();
        addLogMessage("[Primitive] Cone mode: Click to set center, click for base radius, click for height");
    }
}

void LzzCad::onCreateExtrude()
{
    if (m_occView) {
        m_occView->startExtrudeMode();
        addLogMessage("[Feature] Extrude mode: Click to select a sketch, then enter extrude height");
    }
}

void LzzCad::onCreateRevolve()
{
    if (m_occView) {
        m_occView->startRevolveMode();
        addLogMessage("[Feature] Revolve mode: Click to select a sketch, then set revolve parameters");
    }
}

void LzzCad::onCreateSweep()
{
    if (m_occView) {
        m_occView->startSweepMode();
        addLogMessage("[Feature] Sweep mode: Click to select profile, then click to select path");
    }
}

void LzzCad::onBooleanUnion()
{
    performBoolean(BooleanOp::Bool_Union, "Union");
}

void LzzCad::onBooleanCut()
{
    performBoolean(BooleanOp::Bool_Cut, "Cut");
}

void LzzCad::onBooleanIntersect()
{
    performBoolean(BooleanOp::Bool_Intersect, "Intersect");
}

bool LzzCad::performBoolean(BooleanOp op, const QString& opName)
{
    if (!m_occView || !m_geometryModel || !m_modelTree || !m_commandManager)
    {
        addLogMessage("[Boolean] Error: Missing required components");
        return false;
    }

    Handle(AIS_InteractiveContext) ctx = m_occView->getContext();
    if (ctx.IsNull())
    {
        addLogMessage("[Boolean] Error: Context is null");
        return false;
    }

    // Get all selected shapes
    QList<Handle(AIS_InteractiveObject)> selectedObjs = m_occView->getSelectedObjects();
    if (selectedObjs.size() < 2)
    {
        QMessageBox::warning(this, tr("Boolean Operation"),
            tr("Please select at least 2 shapes to perform a boolean operation.\n\n"
               "Current selection: %1 shape(s)").arg(selectedObjs.size()));
        addLogMessage(QString("[Boolean] %1 failed: only %2 shape(s) selected (need 2+)").arg(opName).arg(selectedObjs.size()));
        return false;
    }

    // Get the first two selected shapes as AIS_Shape
    Handle(AIS_Shape) aisShape1 = Handle(AIS_Shape)::DownCast(selectedObjs[0]);
    Handle(AIS_Shape) aisShape2 = Handle(AIS_Shape)::DownCast(selectedObjs[1]);

    if (aisShape1.IsNull() || aisShape2.IsNull())
    {
        addLogMessage("[Boolean] Error: Selected objects are not valid AIS_Shapes");
        return false;
    }

    TopoDS_Shape shape1 = aisShape1->Shape();
    TopoDS_Shape shape2 = aisShape2->Shape();

    if (shape1.IsNull() || shape2.IsNull())
    {
        addLogMessage("[Boolean] Error: Shapes are null");
        return false;
    }

    // Perform the boolean operation with error handling
    TopoDS_Shape resultShape;
    try
    {
        switch (op)
        {
        case BooleanOp::Bool_Union:
        {
            BRepAlgoAPI_Fuse maker(shape1, shape2);
            maker.Build();
            if (!maker.IsDone())
            {
                addLogMessage("[Boolean] Union operation failed");
                QMessageBox::warning(this, tr("Boolean Operation"), tr("Union operation failed."));
                return false;
            }
            resultShape = maker.Shape();
            break;
        }
        case BooleanOp::Bool_Cut:
        {
            BRepAlgoAPI_Cut maker(shape1, shape2);
            maker.Build();
            if (!maker.IsDone())
            {
                addLogMessage("[Boolean] Cut operation failed");
                QMessageBox::warning(this, tr("Boolean Operation"), tr("Cut operation failed."));
                return false;
            }
            resultShape = maker.Shape();

            // Check if the cut actually modified anything (non-overlapping shapes)
            // Compare bounding boxes of original shape1 and result
            if (!resultShape.IsNull())
            {
                Bnd_Box box1, boxResult;
                BRepBndLib::Add(shape1, box1);
                BRepBndLib::Add(resultShape, boxResult);
                if (!box1.IsVoid() && !boxResult.IsVoid())
                {
                    Standard_Real xMin1, yMin1, zMin1, xMax1, yMax1, zMax1;
                    Standard_Real xMinR, yMinR, zMinR, xMaxR, yMaxR, zMaxR;
                    box1.Get(xMin1, yMin1, zMin1, xMax1, yMax1, zMax1);
                    boxResult.Get(xMinR, yMinR, zMinR, xMaxR, yMaxR, zMaxR);
                    double eps = 0.001;
                    if (fabs(xMin1 - xMinR) < eps && fabs(yMin1 - yMinR) < eps && fabs(zMin1 - zMinR) < eps &&
                        fabs(xMax1 - xMaxR) < eps && fabs(yMax1 - yMaxR) < eps && fabs(zMax1 - zMaxR) < eps)
                    {
                        addLogMessage("[Boolean] Cut result is identical to original (shapes do not overlap)");
                        QMessageBox::warning(this, tr("Boolean Operation"),
                            tr("The Cut operation did not modify the shape.\n\n"
                               "The two shapes do not overlap, so nothing was cut away.\n"
                               "Try moving the shapes to overlap each other before cutting."));
                        return false;
                    }
                }
            }
            break;
        }
        case BooleanOp::Bool_Intersect:
        {
            BRepAlgoAPI_Common maker(shape1, shape2);
            maker.Build();
            if (!maker.IsDone())
            {
                addLogMessage("[Boolean] Intersect operation failed");
                QMessageBox::warning(this, tr("Boolean Operation"), tr("Intersect operation failed."));
                return false;
            }
            resultShape = maker.Shape();
            break;
        }
        }
    }
    catch (Standard_Failure& e)
    {
        QString errMsg = QString::fromUtf8(e.GetMessageString());
        addLogMessage(QString("[Boolean] Exception: %1").arg(errMsg));
        QMessageBox::warning(this, tr("Boolean Error"), tr("Boolean operation failed: %1").arg(errMsg));
        return false;
    }

    if (resultShape.IsNull())
    {
        addLogMessage("[Boolean] Result shape is null");
        QMessageBox::warning(this, tr("Boolean Operation"),
            tr("The %1 operation produced an empty result.\n\n"
               "Possible reasons:\n"
               "- Shapes do not overlap (Intersect)\n"
               "- Cut shape completely contains the target (Cut)\n"
               "- Shapes are separate (Union might still work)\n\n"
               "Try selecting different shapes or adjusting their positions.").arg(opName));
        return false;
    }

    // Check if the result has any actual sub-shapes (empty intersection/cut)
    {
        TopExp_Explorer exp(resultShape, TopAbs_SOLID);
        TopExp_Explorer expFace(resultShape, TopAbs_FACE);
        if (!exp.More() && !expFace.More())
        {
            addLogMessage(QString("[Boolean] %1 result has no sub-shapes (empty result)").arg(opName));
            QMessageBox::warning(this, tr("Boolean Operation"),
                tr("The %1 operation produced an empty result.\n\n"
                   "Possible reasons:\n"
                   "- Shapes do not overlap (Intersect)\n"
                   "- Cut shape completely contains the target (Cut)\n"
                   "- Shapes are separate (Union might still work)\n\n"
                   "Try selecting different shapes or adjusting their positions.").arg(opName));
            return false;
        }
    }

    // Save names before removing originals (for model tree cleanup)
    QString name1 = m_geometryModel->getShapeName(aisShape1);
    QString name2 = m_geometryModel->getShapeName(aisShape2);

    // Remove original shapes from context
    ctx->Erase(aisShape1, Standard_False);
    ctx->Erase(aisShape2, Standard_False);

    // Remove from model
    m_geometryModel->removeShape(aisShape1);
    m_geometryModel->removeShape(aisShape2);

    // Remove from model tree
    removeModelTreeItem(name1);
    removeModelTreeItem(name2);

    // Create the new AIS_Shape for the result
    Handle(AIS_Shape) resultAIS = new AIS_Shape(resultShape);
    resultAIS->SetColor(Quantity_NOC_YELLOW);
    resultAIS->SetDisplayMode(AIS_Shaded);
    ctx->Display(resultAIS, Standard_True);
    ctx->ClearSelected(Standard_False);
    ctx->UpdateCurrentViewer();

    // Record in command manager for undo
    if (m_commandManager)
    {
        m_commandManager->executeCommand(new DisplayShapeCommand(ctx, resultAIS,
            QString("Boolean %1").arg(opName)));
    }

    // Add to model
    m_geometryModel->addShape(resultAIS, ShapeType::Model,
        QString("%1_Result").arg(opName));

    // Add to model tree
    QString resultName = m_geometryModel->getShapeNames().back();
    addModelTreeItem(resultName);

    // Auto-select the result in model tree and update PropertyPanel
    if (m_modelTree)
    {
        QTreeWidgetItem* rootItem = m_modelTree->topLevelItem(0);
        if (rootItem)
        {
            for (int i = 0; i < rootItem->childCount(); ++i)
            {
                QTreeWidgetItem* child = rootItem->child(i);
                if (child->text(0) == resultName)
                {
                    m_modelTree->setCurrentItem(child);
                    m_occView->selectShape(resultAIS);
                    if (m_propertyPanel)
                    {
                        m_propertyPanel->updateSelection(resultAIS, resultName, ShapeType::Model);
                    }
                    break;
                }
            }
        }
    }

    addLogMessage(QString("[Boolean] %1 completed: %2 + %3 -> %4")
        .arg(opName).arg(name1).arg(name2).arg(resultName));

    // Fit all to show the new result
    m_occView->fitAll();

    return true;
}

void LzzCad::onDeleteSelected()
{
    if (m_occView && m_commandManager && m_geometryModel && m_modelTree)
    {
        QTreeWidgetItem* selectedItem = m_modelTree->currentItem();
        if (selectedItem && selectedItem->parent())
        {
            QString itemName = selectedItem->text(0);
            
            const auto& shapes = m_geometryModel->getShapes();
            const auto& names = m_geometryModel->getShapeNames();
            
            for (size_t i = 0; i < names.size(); ++i)
            {
                if (names[i] == itemName)
                {
                    QList<Handle(AIS_InteractiveObject)> selectedObjects;
                    selectedObjects.append(shapes[i]);
                    
                    DeleteCommand* cmd = new DeleteCommand(m_occView->getContext(), selectedObjects);
                    m_commandManager->executeCommand(cmd);
                    
                    m_geometryModel->removeShape(shapes[i]);
                    removeModelTreeItem(itemName);
                    
                    addLogMessage(QString("[Tools] Deleted: %1").arg(itemName));
                    break;
                }
            }
        }
        else
        {
            QList<Handle(AIS_InteractiveObject)> selectedObjects = m_occView->getSelectedObjects();
            if (!selectedObjects.isEmpty())
            {
                DeleteCommand* cmd = new DeleteCommand(m_occView->getContext(), selectedObjects);
                m_commandManager->executeCommand(cmd);
                
                for (const Handle(AIS_InteractiveObject)& obj : selectedObjects)
                {
                    Handle(AIS_Shape) aisShape = Handle(AIS_Shape)::DownCast(obj);
                    if (!aisShape.IsNull())
                    {
                        QString name = m_geometryModel->getShapeName(aisShape);
                        m_geometryModel->removeShape(aisShape);
                        removeModelTreeItem(name);
                    }
                }
                
                addLogMessage("[Tools] Deleted selected objects");
            }
            else
            {
                addLogMessage("[Tools] No objects selected");
            }
        }
    }
}

void LzzCad::addModelTreeItem(const QString& name)
{
    if (m_modelTree)
    {
        QTreeWidgetItem* rootItem = m_modelTree->topLevelItem(0);
        if (rootItem)
        {
            QTreeWidgetItem* item = new QTreeWidgetItem(rootItem);
            item->setText(0, name);
            item->setIcon(1, m_showIcon);
            rootItem->setExpanded(true);
            addLogMessage(QString("[Model Tree] Added: %1").arg(name));
        }
    }
}

void LzzCad::removeModelTreeItem(const QString& name)
{
    if (m_modelTree)
    {
        QTreeWidgetItem* rootItem = m_modelTree->topLevelItem(0);
        if (rootItem)
        {
            for (int i = 0; i < rootItem->childCount(); ++i)
            {
                QTreeWidgetItem* child = rootItem->child(i);
                if (child->text(0) == name)
                {
                    delete rootItem->takeChild(i);
                    addLogMessage(QString("[Model Tree] Removed: %1").arg(name));
                    break;
                }
            }
        }
    }
}

void LzzCad::clearModelTree()
{
    if (m_modelTree)
    {
        QTreeWidgetItem* rootItem = m_modelTree->topLevelItem(0);
        if (rootItem)
        {
            while (rootItem->childCount() > 0)
            {
                delete rootItem->takeChild(0);
            }
        }
        addLogMessage("[Model Tree] All shapes cleared");
    }
}

void LzzCad::onModelTreeItemClicked(QTreeWidgetItem* item, int column)
{
    if (!item || !m_occView || !m_geometryModel)
        return;
    
    QString itemName = item->text(0);
    if (itemName == "Assembly")
        return;
    
    if (column == 1)
    {
        toggleShapeVisibility(item, column);
    }
    else if (column == 0)
    {
        const auto& shapes = m_geometryModel->getShapes();
        const auto& names = m_geometryModel->getShapeNames();
        const auto& types = m_geometryModel->getShapeTypes();
        
        for (size_t i = 0; i < names.size(); ++i)
        {
            if (names[i] == itemName)
            {
                m_occView->selectShape(shapes[i]);
                if (m_propertyPanel)
                {
                    m_propertyPanel->updateSelection(shapes[i], names[i], types[i]);
                }
                addLogMessage(QString("[Model Tree] Selected: %1").arg(itemName));
                break;
            }
        }
    }
}

void LzzCad::toggleShapeVisibility(QTreeWidgetItem* item, int column)
{
    if (!item || !m_occView || !m_geometryModel)
        return;
    
    QString itemName = item->text(0);
    const auto& shapes = m_geometryModel->getShapes();
    const auto& names = m_geometryModel->getShapeNames();
    
    for (size_t i = 0; i < names.size(); ++i)
    {
        if (names[i] == itemName)
        {
            bool isVisible = m_occView->isShapeVisible(shapes[i]);
            if (isVisible)
            {
                m_occView->hideShape(shapes[i]);
                item->setIcon(1, m_hideIcon);
                addLogMessage(QString("[Model Tree] Hidden: %1").arg(itemName));
            }
            else
            {
                m_occView->showShape(shapes[i]);
                item->setIcon(1, m_showIcon);
                addLogMessage(QString("[Model Tree] Shown: %1").arg(itemName));
            }
            break;
        }
    }
}

void LzzCad::onSendMessage()
{
    QString message = m_chatInput->text().trimmed();
    if (message.isEmpty() || m_isProcessing) return;
    
    m_chatInput->clear();
    m_chatHistory->append(QString("<b>You:</b> %1").arg(message));
    
    m_isProcessing = true;
    m_sendButton->setEnabled(false);
    m_chatInput->setEnabled(false);
    
    addLogMessage(QString("[AI] Sending message: %1").arg(message));
    
    QNetworkRequest request{QUrl(m_apiUrl)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());
    
    QJsonArray messages;
    
    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = QString(R"(
You are a CAD assistant for LzzCad software. You can help users with 3D modeling tasks.

Available tools:
1. open_file - Open a STEP/BREP/IGES model file
   Parameters:
   - file_path: string (full path to the file, e.g., "C:/models/part.step")
   
2. save_file - Save current model to file
   Parameters:
   - file_path: string (full path to the file)

3. clear_all - Clear all shapes from the view

4. fit_all - Adjust view to show all shapes

5. view_top - Switch to top view
6. view_bottom - Switch to bottom view
7. view_front - Switch to front view
8. view_back - Switch to back view
9. view_left - Switch to left view
10. view_right - Switch to right view

11. shaded - Enable shaded display mode
12. wireframe - Enable wireframe display mode

13. sketch_point - Start interactive point drawing mode (click once to place a point)
14. sketch_line - Start interactive line drawing mode (click to set start point, click again to set end point)
15. sketch_circle - Start interactive circle drawing mode (click to set center, click again to set radius)
16. sketch_arc - Start interactive arc drawing mode (click to set center, click for start point, click for end point)
17. sketch_cancel - Cancel current sketch mode and return to normal view

18. draw_point - Draw a point at specified coordinates
   Parameters:
   - x: number (X coordinate)
   - y: number (Y coordinate)

19. draw_line - Draw a line between two points
   Parameters:
   - x1: number (Start point X coordinate)
   - y1: number (Start point Y coordinate)
   - x2: number (End point X coordinate)
   - y2: number (End point Y coordinate)

20. draw_polyline - Draw a polyline through multiple points (open path)
   Parameters:
   - points: array of {x, y} objects (List of points in order)

21. draw_polygon - Draw a closed polygon through multiple points
   Parameters:
   - points: array of {x, y} objects (List of vertices in order, automatically closes the shape)

22. draw_circle - Draw a circle with center and radius
   Parameters:
   - cx: number (Center X coordinate)
   - cy: number (Center Y coordinate)
   - radius: number (Circle radius)

23. draw_arc - Draw an arc with center, radius, start angle and sweep angle
   Parameters:
   - cx: number (Center X coordinate)
   - cy: number (Center Y coordinate)
   - radius: number (Arc radius)
   - start_angle: number (Start angle in degrees, 0° = right along +X axis, 90° = up along +Y axis)
   - sweep_angle: number (Sweep angle in degrees, positive = counterclockwise, negative = clockwise)
   
   Examples:
   - Smile mouth (upward curve on screen): { "cx": 0, "cy": -10, "radius": 25, "start_angle": 210, "sweep_angle": 120 }
   - Frown mouth (downward curve on screen): { "cx": 0, "cy": -10, "radius": 25, "start_angle": 210, "sweep_angle": -120 }
   - Quarter-circle from right to top: { "cx": 0, "cy": 0, "radius": 50, "start_angle": 0, "sweep_angle": 90 }

When user asks to open a file, use the open_file tool with the file path.
When user asks to save a file, use the save_file tool with the file path.
For view operations, use the appropriate view tool.
For display mode, use shaded or wireframe tool.
For sketching, use the appropriate sketch tool.

Format your response as a JSON object or JSON array of objects with the following structure:

Single tool call:
{
  "tool": "tool_name",
  "parameters": {
    "param1": "value1",
    "param2": "value2"
  },
  "message": "Optional human-readable message to display"
}

Multiple tool calls (for complex shapes like smiley faces, stars with multiple parts, etc.):
[
  { "tool": "draw_circle", "parameters": { "cx": 0, "cy": 0, "radius": 50 }, "message": "绘制脸" },
  { "tool": "draw_circle", "parameters": { "cx": -20, "cy": 15, "radius": 8 }, "message": "绘制左眼" },
  { "tool": "draw_circle", "parameters": { "cx": 20, "cy": 15, "radius": 8 }, "message": "绘制右眼" },
  { "tool": "draw_arc", "parameters": { "cx": 0, "cy": -10, "radius": 25, "start_angle": 210, "sweep_angle": 120 }, "message": "绘制微笑嘴巴" }
]

Frown face example (with downward curve mouth on screen):
[
  { "tool": "draw_circle", "parameters": { "cx": 0, "cy": 0, "radius": 50 }, "message": "绘制脸" },
  { "tool": "draw_circle", "parameters": { "cx": -20, "cy": 15, "radius": 8 }, "message": "绘制左眼" },
  { "tool": "draw_circle", "parameters": { "cx": 20, "cy": 15, "radius": 8 }, "message": "绘制右眼" },
  { "tool": "draw_arc", "parameters": { "cx": 0, "cy": -10, "radius": 25, "start_angle": 210, "sweep_angle": -120 }, "message": "绘制哭脸嘴巴" }
]

If no tool is needed, respond normally as a chat message without JSON format.
)").trimmed();
    messages.append(systemMessage);
    
    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = message;
    messages.append(userMessage);
    
    QJsonObject requestBody;
    requestBody["model"] = "deepseek-chat";
    requestBody["messages"] = messages;
    requestBody["temperature"] = 0.7;
    requestBody["max_tokens"] = 2048;
    
    QJsonDocument doc(requestBody);
    m_networkManager->post(request, doc.toJson());
}

void LzzCad::onApiResponse(QNetworkReply* reply)
{
    m_isProcessing = false;
    m_sendButton->setEnabled(true);
    m_chatInput->setEnabled(true);
    
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMessage = QString("API Error: %1").arg(reply->errorString());
        m_chatHistory->append(QString("<b>AI:</b> <font color='red'>%1</font>").arg(errorMessage));
        addLogMessage(QString("[AI] Error: %1").arg(errorMessage));
        reply->deleteLater();
        return;
    }
    
    QByteArray responseData = reply->readAll();
    reply->deleteLater();
    
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    if (!doc.isObject()) {
        m_chatHistory->append("<b>AI:</b> Invalid response format");
        addLogMessage("[AI] Invalid response format");
        return;
    }
    
    QJsonObject response = doc.object();
    if (response.contains("choices")) {
        QJsonArray choices = response["choices"].toArray();
        if (!choices.isEmpty()) {
            QJsonObject choice = choices[0].toObject();
            QJsonObject message = choice["message"].toObject();
            QString content = message["content"].toString();
            
            m_chatHistory->append(QString("<b>AI:</b> %1").arg(content));
            addLogMessage(QString("[AI] Response received: %1").arg(content.left(50) + "..."));
            
            parseAndExecuteTool(content);
        }
    } else {
        m_chatHistory->append("<b>AI:</b> No response from server");
        addLogMessage("[AI] No response from server");
    }
}

void LzzCad::parseAndExecuteTool(const QString& content)
{
    QString cleanContent = content;
    
    cleanContent.remove("```json");
    cleanContent.remove("```");
    cleanContent.remove("\"\"\"");
    
    QString jsonStr = extractJsonFromText(cleanContent);
    if (jsonStr.isEmpty()) {
        addLogMessage("[AI] Cannot find valid JSON in response");
        return;
    }
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        addLogMessage(QString("[AI] JSON parse error: %1 at offset %2").arg(parseError.errorString()).arg(parseError.offset));
        addLogMessage(QString("[AI] Raw JSON: %1").arg(jsonStr.left(200)));
        return;
    }
    
    if (doc.isArray()) {
        QJsonArray toolsArray = doc.array();
        addLogMessage(QString("[AI] Executing %1 tools").arg(toolsArray.size()));
        for (const QJsonValue& val : toolsArray) {
            if (val.isObject()) {
                processToolCall(val.toObject());
            }
        }
        
        bool hasDrawTool = false;
        for (const QJsonValue& val : toolsArray) {
            if (val.isObject()) {
                QString toolName = val.toObject()["tool"].toString();
                if (toolName.startsWith("draw_") || toolName.startsWith("sketch_")) {
                    hasDrawTool = true;
                    break;
                }
            }
        }
        
        if (hasDrawTool && m_occViewModel) {
            m_occViewModel->fitAll();
        }
    } else if (doc.isObject()) {
        processToolCall(doc.object());
        
        QString toolName = doc.object()["tool"].toString();
        if ((toolName.startsWith("draw_") || toolName.startsWith("sketch_")) && m_occViewModel) {
            m_occViewModel->fitAll();
        }
    }
}

QString LzzCad::extractJsonFromText(const QString& text)
{
    int braceCount = 0;
    int bracketCount = 0;
    int startIdx = -1;
    int endIdx = -1;
    bool inString = false;
    QChar prevChar = '\0';
    
    for (int i = 0; i < text.length(); ++i) {
        QChar c = text[i];
        
        if (c == '"' && prevChar != '\\') {
            inString = !inString;
        }
        
        if (inString) {
            prevChar = c;
            continue;
        }
        
        if (c == '[') {
            bracketCount++;
            if (startIdx == -1) {
                startIdx = i;
            }
        } else if (c == ']') {
            bracketCount--;
            if (bracketCount == 0 && startIdx != -1) {
                endIdx = i + 1;
                break;
            }
        } else if (c == '{') {
            braceCount++;
            if (startIdx == -1 && bracketCount == 0) {
                startIdx = i;
            }
        } else if (c == '}') {
            braceCount--;
            if (braceCount == 0 && startIdx != -1 && bracketCount == 0) {
                endIdx = i + 1;
                break;
            }
        }
        
        prevChar = c;
    }
    
    if (startIdx != -1 && endIdx != -1 && startIdx < endIdx) {
        return text.mid(startIdx, endIdx - startIdx);
    }
    
    return QString();
}

void LzzCad::processToolCall(const QJsonObject& toolObj)
{
    QString toolName = toolObj["tool"].toString();
    QJsonObject parameters = toolObj["parameters"].toObject();
    QString message = toolObj["message"].toString();
    
    if (!message.isEmpty()) {
        m_chatHistory->append(QString("<b>AI:</b> %1").arg(message));
    }
    
    addLogMessage(QString("[AI] Executing tool: %1").arg(toolName));
    
    if (toolName == "open_file") {
        QString filePath = parameters["file_path"].toString();
        
        if (!filePath.isEmpty() && m_occViewModel) {
            m_chatHistory->append(QString("<b>System:</b> Opening file %1").arg(filePath));
            m_occViewModel->openFile(filePath);
        }
    }
    else if (toolName == "save_file") {
        QString filePath = parameters["file_path"].toString();
        
        if (!filePath.isEmpty() && m_occViewModel) {
            m_chatHistory->append(QString("<b>System:</b> Saving to %1").arg(filePath));
            m_occViewModel->saveFile(filePath);
        }
    }
    else if (toolName == "clear_all") {
        if (m_occViewModel) {
            m_chatHistory->append("<b>System:</b> Clearing all shapes");
            m_occViewModel->clearAllShapes();
        }
    }
    else if (toolName == "fit_all") {
        if (m_occViewModel) {
            m_chatHistory->append("<b>System:</b> Fitting view to all shapes");
            m_occViewModel->fitAll();
        }
    }
    else if (toolName == "view_top") {
        if (m_occViewModel) {
            m_occViewModel->viewTop();
        }
    }
    else if (toolName == "view_bottom") {
        if (m_occViewModel) {
            m_occViewModel->viewBottom();
        }
    }
    else if (toolName == "view_front") {
        if (m_occViewModel) {
            m_occViewModel->viewFront();
        }
    }
    else if (toolName == "view_back") {
        if (m_occViewModel) {
            m_occViewModel->viewBack();
        }
    }
    else if (toolName == "view_left") {
        if (m_occViewModel) {
            m_occViewModel->viewLeft();
        }
    }
    else if (toolName == "view_right") {
        if (m_occViewModel) {
            m_occViewModel->viewRight();
        }
    }
    else if (toolName == "set_shaded") {
        if (m_occViewModel) {
            m_occViewModel->setShaded(true);
        }
    }
    else if (toolName == "set_wireframe") {
        if (m_occViewModel) {
            m_occViewModel->setShaded(false);
        }
    }
    else if (toolName == "sketch_point") {
        onSketchPoint();
    }
    else if (toolName == "sketch_line") {
        onSketchLine();
    }
    else if (toolName == "sketch_circle") {
        onSketchCircle();
    }
    else if (toolName == "sketch_arc") {
        onSketchArc();
    }
    else if (toolName == "sketch_cancel") {
        if (m_occView) {
            m_occView->cancelSketch();
        }
    }
    else if (toolName == "draw_point") {
        if (m_occView) {
            double x = parameters["x"].toDouble();
            double y = parameters["y"].toDouble();
            m_occView->drawPoint(x, y);
        }
    }
    else if (toolName == "draw_line") {
        if (m_occView) {
            double x1 = parameters["x1"].toDouble();
            double y1 = parameters["y1"].toDouble();
            double x2 = parameters["x2"].toDouble();
            double y2 = parameters["y2"].toDouble();
            m_occView->drawLine(x1, y1, x2, y2);
        }
    }
    else if (toolName == "draw_polyline") {
        if (m_occView) {
            QJsonArray pointsArray = parameters["points"].toArray();
            QVector<QPair<double, double>> points;
            for (const QJsonValue& val : pointsArray) {
                QJsonObject pointObj = val.toObject();
                double x = pointObj["x"].toDouble();
                double y = pointObj["y"].toDouble();
                points.append(QPair<double, double>(x, y));
            }
            m_occView->drawPolyline(points);
        }
    }
    else if (toolName == "draw_polygon") {
        if (m_occView) {
            QJsonArray pointsArray = parameters["points"].toArray();
            QVector<QPair<double, double>> points;
            for (const QJsonValue& val : pointsArray) {
                QJsonObject pointObj = val.toObject();
                double x = pointObj["x"].toDouble();
                double y = pointObj["y"].toDouble();
                points.append(QPair<double, double>(x, y));
            }
            m_occView->drawPolygon(points);
        }
    }
    else if (toolName == "draw_circle") {
        if (m_occView) {
            double cx = parameters["cx"].toDouble();
            double cy = parameters["cy"].toDouble();
            double radius = parameters["radius"].toDouble();
            m_occView->drawCircle(cx, cy, radius);
        }
    }
    else if (toolName == "draw_arc") {
        if (m_occView) {
            double cx = parameters["cx"].toDouble();
            double cy = parameters["cy"].toDouble();
            double radius = parameters["radius"].toDouble();
            double startAngle = parameters["start_angle"].toDouble();
            double sweepAngle = parameters["sweep_angle"].toDouble();
            m_occView->drawArc(cx, cy, radius, startAngle, sweepAngle);
        }
    }
}

void LzzCad::testApiKey()
{
    if (m_apiKey.isEmpty()) {
        return;
    }
    
    addLogMessage("[AI] Testing API key...");
    
    QNetworkRequest request{QUrl(m_apiUrl)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());
    
    QJsonObject testMessage;
    testMessage["role"] = "user";
    testMessage["content"] = "Hello";
    
    QJsonArray messages;
    messages.append(testMessage);
    
    QJsonObject requestBody;
    requestBody["model"] = "deepseek-chat";
    requestBody["messages"] = messages;
    requestBody["max_tokens"] = 10;
    
    QJsonDocument doc(requestBody);
    QNetworkReply* reply = m_networkManager->post(request, doc.toJson());
    
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            QString errorMessage = QString("[AI] API test failed: %1").arg(reply->errorString());
            addLogMessage(errorMessage);
            m_chatHistory->append(QString("<b>AI:</b> <font color='red'>API test failed: %1</font>").arg(reply->errorString()));
        } else {
            addLogMessage("[AI] API key is valid!");
            m_chatHistory->append("<b>AI:</b> <font color='green'>API test successful! Ready to chat.</font>");
        }
        reply->deleteLater();
    });
}

void LzzCad::addLogMessage(const QString& message)
{
    if (m_logEdit) {
        m_logEdit->append(message);
    }
}

void LzzCad::updatePositionLabel(const QPoint& ptScreen)
{
    if (!m_occView || m_occView->getView().IsNull()) {
        return;
    }
    
    Standard_Real X, Y, Z;
    m_occView->getView()->Convert(ptScreen.x(), ptScreen.y(), X, Y, Z);
    
    gp_Pnt pt1(X, Y, Z);
    
    const Handle(Graphic3d_Camera)& c = m_occView->getView()->Camera();
    const gp_Dir& v = c->Direction();
    
    Handle(Geom_Curve) aLine = new Geom_Line(pt1, v);
    
    gp_Pln prjPln(gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)));
    Handle(Geom_Surface) aPlane = new Geom_Plane(prjPln);
    GeomAPI_IntCS aIntCS(aLine, aPlane);
    
    if (aIntCS.IsDone() && aIntCS.NbPoints() > 0) {
        gp_Pnt aPnt = aIntCS.Point(1);
        m_positionLabel->setText(QString("X: %1   Y: %2   Z: %3").arg(aPnt.X(), 0, 'f', 3).arg(aPnt.Y(), 0, 'f', 3).arg(aPnt.Z(), 0, 'f', 3));
    } else {
        m_positionLabel->setText(QString("X: %1   Y: %2   Z: %3").arg(X, 0, 'f', 3).arg(Y, 0, 'f', 3).arg(Z, 0, 'f', 3));
    }
}
