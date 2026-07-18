#pragma once

#include <QtWidgets/QMainWindow>
#include <QDockWidget>
#include <QTreeWidget>
#include <QTextEdit>
#include <QAction>
#include <QIcon>
#include <QLabel>
#include <QString>
#include "View/PropertyPanel.h"
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

#include "ui_LzzCad.h"
#include "SARibbon.h"
#include "view.h"
#include "Model/GeometryModel.h"
#include "ViewModel/OccViewModel.h"
#include "Command/Command.h"

class LzzCad : public SARibbonMainWindow
{
    Q_OBJECT

public:
    LzzCad(QWidget* parent = nullptr);
    ~LzzCad();

private:
    void createRibbon();
    void createCentralWidget();
    void createDockWidgets();
    
    void createModelTreeDock();
    void createPropertyDock();
    void createLogDock();
    
    // Ribbon category creation methods
    void createFileCategory(SARibbonBar* ribbon);
    void createSketchCategory(SARibbonBar* ribbon);
    void createModelCategory(SARibbonBar* ribbon);
    void createViewCategory(SARibbonBar* ribbon);
    void createToolsCategory(SARibbonBar* ribbon);
    void createCAMCategory(SARibbonBar* ribbon);
    void createHelpCategory(SARibbonBar* ribbon);
    
    void setupWindowIcon();
    QIcon loadIcon(const QString& iconName);
    
    void addModelTreeItem(const QString& name);
    void removeModelTreeItem(const QString& name);
    void clearModelTree();

private slots:
    void onViewFitAll();
    void onViewZoom();
    void onViewPan();
    void onViewRotate();
    void onViewTop();
    void onViewBottom();
    void onViewLeft();
    void onViewRight();
    void onViewFront();
    void onViewBack();
    void onViewShaded();
    void onViewWireframe();
    
    void onFileOpen();
    void onFileSave();
    void onClearAll();
    
    void onUndo();
    void onRedo();
    
    // Sketch drawing slots
    void onSketchPoint();
    void onSketchLine();
    void onSketchCircle();
    void onSketchArc();
    
    // Primitive creation slots
    void onCreateBox();
    void onCreateSphere();
    void onCreateCylinder();
    void onCreateCone();
    
    // Feature creation slots
    void onCreateExtrude();
    void onCreateRevolve();
    void onCreateSweep();
    
    void onDeleteSelected();
    
    void onModelTreeItemClicked(QTreeWidgetItem* item, int column);
    
    void onSendMessage();
    void onApiResponse(QNetworkReply* reply);
    void addLogMessage(const QString& message);
    void parseAndExecuteTool(const QString& content);
    void processToolCall(const QJsonObject& toolObj);
    QString extractJsonFromText(const QString& text);
    void testApiKey();
    void updatePositionLabel(const QPoint& ptScreen);

private:
    Ui::LzzCadClass ui;

    QTreeWidget* m_modelTree;
    PropertyPanel* m_propertyPanel;
    QTextEdit* m_logEdit;
    QLabel* m_positionLabel;
    
    // AI chat members
    QTextEdit* m_chatHistory;
    QLineEdit* m_chatInput;
    QPushButton* m_sendButton;
    QNetworkAccessManager* m_networkManager;
    QString m_apiKey;
    QString m_apiUrl;
    bool m_isProcessing;
    
    // MVVM components
    GeometryModel* m_geometryModel;
    OccViewModel* m_occViewModel;
    OccView* m_occView;
    
    // Command manager for undo/redo
    CommandManager* m_commandManager;
    
    // Model tree icons
    QIcon m_showIcon;
    QIcon m_hideIcon;
    
    void toggleShapeVisibility(QTreeWidgetItem* item, int column);
};