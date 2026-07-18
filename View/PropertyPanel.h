#pragma once

#include <QWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QTreeWidget>
#include <QHeaderView>
#include <QScrollArea>
#include <QFormLayout>
#include <QRegExp>
#include <QColorDialog>
#include <AIS_Shape.hxx>
#include <TopoDS_Shape.hxx>
#include <Quantity_Color.hxx>
#include "../Model/GeometryModel.h"

class CollapsiblePanel : public QWidget
{
    Q_OBJECT

public:
    explicit CollapsiblePanel(const QString& title, QWidget* parent = nullptr)
        : QWidget(parent), m_isExpanded(true)
    {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        m_header = new QPushButton(title);
        m_header->setStyleSheet(
            "QPushButton { "
            "  background-color: #f5f5f5; "
            "  border: 1px solid #e0e0e0; "
            "  border-radius: 6px; "
            "  padding: 8px 12px; "
            "  text-align: left; "
            "  font-weight: 600; "
            "  font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif; "
            "  font-size: 14px; "
            "  color: #333333; "
            "} "
            "QPushButton:hover { "
            "  background-color: #e8e8e8; "
            "} "
            "QPushButton:pressed { "
            "  background-color: #dcdcdc; "
            "}"
        );
        connect(m_header, &QPushButton::clicked, this, &CollapsiblePanel::toggle);
        mainLayout->addWidget(m_header);

        m_content = new QWidget();
        m_contentLayout = new QVBoxLayout(m_content);
        m_contentLayout->setContentsMargins(8, 8, 8, 8);
        m_contentLayout->setSpacing(6);
        mainLayout->addWidget(m_content);

        setLayout(mainLayout);
    }

    QVBoxLayout* contentLayout() { return m_contentLayout; }

    void setExpanded(bool expanded)
    {
        m_isExpanded = expanded;
        m_content->setVisible(expanded);
        updateHeader();
    }

    bool isExpanded() const { return m_isExpanded; }

private slots:
    void toggle()
    {
        setExpanded(!m_isExpanded);
    }

private:
    void updateHeader()
    {
        QString title = m_header->text();
        title = title.replace(QRegExp("^[\\-\\+] "), "");
        m_header->setText(QString("%1 %2").arg(m_isExpanded ? "-" : "+").arg(title));
    }

    QPushButton* m_header;
    QWidget* m_content;
    QVBoxLayout* m_contentLayout;
    bool m_isExpanded;
};

class PropertyPanel : public QWidget
{
    Q_OBJECT

signals:
    void nameChanged(const QString& name);
    void applyTransform(double x, double y, double z, double rx, double ry, double rz);
    void colorChanged(const Handle(AIS_Shape)& shape, const Quantity_Color& color);

public:
    explicit PropertyPanel(QWidget* parent = nullptr);

    void clear();
    void updateSelection(const Handle(AIS_Shape)& shape, const QString& name, ShapeType type);

private:
    QVBoxLayout* m_mainLayout;
    QScrollArea* m_scrollArea;
    QWidget* m_scrollContent;

    CollapsiblePanel* m_generalPanel;
    CollapsiblePanel* m_shapeInspectorPanel;
    CollapsiblePanel* m_transformPanel;
    CollapsiblePanel* m_sketchPanel;

    QTreeWidget* m_shapeTree;
    
    QLineEdit* m_nameEdit;
    QLineEdit* m_layerEdit;
    QPushButton* m_colorButton;
    Handle(AIS_Shape) m_currentShape;
    
    QDoubleSpinBox* m_posX;
    QDoubleSpinBox* m_posY;
    QDoubleSpinBox* m_posZ;
    QDoubleSpinBox* m_rotX;
    QDoubleSpinBox* m_rotY;
    QDoubleSpinBox* m_rotZ;

    QFormLayout* m_sketchLayout;

    void populateShapeInspector(const TopoDS_Shape& shape);
    void populateSketchProperties(const Handle(AIS_Shape)& shape, ShapeType type);
    void clearSketchLayout();

private slots:
    void onNameChanged(const QString& text);
    void onTransformChanged();
    void onApplyTransform();
    void onColorClicked();
};
