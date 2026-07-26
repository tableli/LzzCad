#define _USE_MATH_DEFINES
#include <cmath>

#include "PropertyPanel.h"
#include "../Model/GeometryModel.h"
#include <TopExp_Explorer.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Vertex.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Line.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <gp_Elips.hxx>
#include <Geom_Ellipse.hxx>
#include <Standard_Type.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <QFormLayout>
#include <QScrollArea>

PropertyPanel::PropertyPanel(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(4);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet(
        "QScrollArea { "
        "  border: none; "
        "} "
        "QScrollBar:vertical { "
        "  width: 8px; "
        "  background: #f0f0f0; "
        "} "
        "QScrollBar::handle:vertical { "
        "  background: #c0c0c0; "
        "  border-radius: 4px; "
        "} "
    );

    m_scrollContent = new QWidget();
    m_scrollContent->setStyleSheet("background-color: #ffffff;");
    m_mainLayout = new QVBoxLayout(m_scrollContent);
    m_mainLayout->setContentsMargins(4, 4, 4, 4);
    m_mainLayout->setSpacing(4);

    m_generalPanel = new CollapsiblePanel("General");
    m_mainLayout->addWidget(m_generalPanel);

    QFormLayout* generalLayout = new QFormLayout();
    generalLayout->setSpacing(4);
    m_generalPanel->contentLayout()->addLayout(generalLayout);

    QString labelStyle = 
        "QLabel { "
        "  font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif; "
        "  font-size: 13px; "
        "  color: #555555; "
        "  font-weight: 500; "
        "}";

    QString lineEditStyle = 
        "QLineEdit { "
        "  border: 1px solid #c0c0c0; "
        "  border-radius: 4px; "
        "  padding: 5px 8px; "
        "  font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif; "
        "  font-size: 13px; "
        "  color: #333333; "
        "  background-color: #ffffff; "
        "} "
        "QLineEdit:hover { "
        "  border-color: #999999; "
        "} "
        "QLineEdit:focus { "
        "  border-color: #4a90d9; "
        "  outline: none; "
        "}";

    QLabel* nameLabel = new QLabel("Name:");
    nameLabel->setStyleSheet(labelStyle);
    m_nameEdit = new QLineEdit();
    m_nameEdit->setStyleSheet(lineEditStyle);
    connect(m_nameEdit, &QLineEdit::textChanged, this, &PropertyPanel::onNameChanged);
    generalLayout->addRow(nameLabel, m_nameEdit);

    QLabel* layerLabel = new QLabel("Layer:");
    layerLabel->setStyleSheet(labelStyle);
    m_layerEdit = new QLineEdit("Default");
    m_layerEdit->setStyleSheet(lineEditStyle);
    generalLayout->addRow(layerLabel, m_layerEdit);

    QLabel* colorLabel = new QLabel("Color:");
    colorLabel->setStyleSheet(labelStyle);
    m_colorButton = new QPushButton();
    m_colorButton->setFixedSize(60, 28);
    m_colorButton->setStyleSheet(
        "QPushButton { "
        "  border: 1px solid #c0c0c0; "
        "  border-radius: 4px; "
        "  padding: 0; "
        "  background-color: transparent; "
        "} "
        "QPushButton:hover { "
        "  border-color: #999999; "
        "} "
        "QPushButton:focus { "
        "  border-color: #4a90d9; "
        "  outline: none; "
        "}");
    
    QPixmap pixmap(56, 24);
    pixmap.fill(QColor(255, 204, 0));
    m_colorButton->setIcon(QIcon(pixmap));
    m_colorButton->setIconSize(QSize(56, 24));
    
    connect(m_colorButton, &QPushButton::clicked, this, &PropertyPanel::onColorClicked);
    generalLayout->addRow(colorLabel, m_colorButton);

    m_shapeInspectorPanel = new CollapsiblePanel("Shape Inspector");
    m_mainLayout->addWidget(m_shapeInspectorPanel);

    m_shapeTree = new QTreeWidget();
    m_shapeTree->setHeaderLabels(QStringList() << "Type" << "Count");
    m_shapeTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_shapeTree->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_shapeTree->header()->resizeSection(1, 60);
    m_shapeTree->setStyleSheet(
        "QTreeWidget { "
        "  border: 1px solid #d0d0d0; "
        "  border-radius: 3px; "
        "  background-color: #fafafa; "
        "} "
        "QTreeWidget::item { "
        "  padding: 2px 4px; "
        "}"
    );
    m_shapeInspectorPanel->contentLayout()->addWidget(m_shapeTree);

    m_transformPanel = new CollapsiblePanel("Transform");
    m_mainLayout->addWidget(m_transformPanel);

    QString spinBoxStyle = 
        "QDoubleSpinBox { "
        "  border: 1px solid #c0c0c0; "
        "  border-radius: 4px; "
        "  padding: 5px 8px; "
        "  font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif; "
        "  font-size: 13px; "
        "  color: #333333; "
        "  background-color: #ffffff; "
        "} "
        "QDoubleSpinBox:hover { "
        "  border-color: #999999; "
        "} "
        "QDoubleSpinBox:focus { "
        "  border-color: #4a90d9; "
        "  outline: none; "
        "} "
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { "
        "  background-color: #f5f5f5; "
        "  border: none; "
        "  width: 18px; "
        "} "
        "QDoubleSpinBox::up-button:hover, QDoubleSpinBox::down-button:hover { "
        "  background-color: #e5e5e5; "
        "}";

    QFormLayout* transformLayout = new QFormLayout();
    transformLayout->setSpacing(6);
    m_transformPanel->contentLayout()->addLayout(transformLayout);

    QLabel* posXLabel = new QLabel("Position X:");
    posXLabel->setStyleSheet(labelStyle);
    m_posX = new QDoubleSpinBox();
    m_posX->setRange(-9999.0, 9999.0);
    m_posX->setDecimals(3);
    m_posX->setSingleStep(0.1);
    m_posX->setStyleSheet(spinBoxStyle);
    connect(m_posX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertyPanel::onTransformChanged);
    transformLayout->addRow(posXLabel, m_posX);

    QLabel* posYLabel = new QLabel("Position Y:");
    posYLabel->setStyleSheet(labelStyle);
    m_posY = new QDoubleSpinBox();
    m_posY->setRange(-9999.0, 9999.0);
    m_posY->setDecimals(3);
    m_posY->setSingleStep(0.1);
    m_posY->setStyleSheet(spinBoxStyle);
    connect(m_posY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertyPanel::onTransformChanged);
    transformLayout->addRow(posYLabel, m_posY);

    QLabel* posZLabel = new QLabel("Position Z:");
    posZLabel->setStyleSheet(labelStyle);
    m_posZ = new QDoubleSpinBox();
    m_posZ->setRange(-9999.0, 9999.0);
    m_posZ->setDecimals(3);
    m_posZ->setSingleStep(0.1);
    m_posZ->setStyleSheet(spinBoxStyle);
    connect(m_posZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertyPanel::onTransformChanged);
    transformLayout->addRow(posZLabel, m_posZ);

    QLabel* rotXLabel = new QLabel("Rotation X:");
    rotXLabel->setStyleSheet(labelStyle);
    m_rotX = new QDoubleSpinBox();
    m_rotX->setRange(-180.0, 180.0);
    m_rotX->setDecimals(1);
    m_rotX->setSingleStep(1.0);
    m_rotX->setStyleSheet(spinBoxStyle);
    connect(m_rotX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertyPanel::onTransformChanged);
    transformLayout->addRow(rotXLabel, m_rotX);

    QLabel* rotYLabel = new QLabel("Rotation Y:");
    rotYLabel->setStyleSheet(labelStyle);
    m_rotY = new QDoubleSpinBox();
    m_rotY->setRange(-180.0, 180.0);
    m_rotY->setDecimals(1);
    m_rotY->setSingleStep(1.0);
    m_rotY->setStyleSheet(spinBoxStyle);
    connect(m_rotY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertyPanel::onTransformChanged);
    transformLayout->addRow(rotYLabel, m_rotY);

    QLabel* rotZLabel = new QLabel("Rotation Z:");
    rotZLabel->setStyleSheet(labelStyle);
    m_rotZ = new QDoubleSpinBox();
    m_rotZ->setRange(-180.0, 180.0);
    m_rotZ->setDecimals(1);
    m_rotZ->setSingleStep(1.0);
    m_rotZ->setStyleSheet(spinBoxStyle);
    connect(m_rotZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertyPanel::onTransformChanged);
    transformLayout->addRow(rotZLabel, m_rotZ);

    QPushButton* applyButton = new QPushButton("Apply");
    applyButton->setStyleSheet(
        "QPushButton { "
        "  background-color: #4a90d9; "
        "  border: none; "
        "  border-radius: 4px; "
        "  padding: 6px 12px; "
        "  color: white; "
        "  font-weight: bold; "
        "  font-size: 13px; "
        "} "
        "QPushButton:hover { "
        "  background-color: #3a7bc8; "
        "} "
        "QPushButton:pressed { "
        "  background-color: #2d6ab3; "
        "}"
    );
    connect(applyButton, &QPushButton::clicked, this, &PropertyPanel::onApplyTransform);
    transformLayout->addRow("", applyButton);

    m_sketchPanel = new CollapsiblePanel("Sketch Properties");
    m_mainLayout->addWidget(m_sketchPanel);

    m_sketchLayout = new QFormLayout();
    m_sketchLayout->setSpacing(4);
    m_sketchPanel->contentLayout()->addLayout(m_sketchLayout);

    m_mainLayout->addStretch();

    m_scrollArea->setWidget(m_scrollContent);
    mainLayout->addWidget(m_scrollArea);

    setLayout(mainLayout);
}

void PropertyPanel::clear()
{
    m_nameEdit->clear();
    m_layerEdit->setText("Default");
    m_shapeTree->clear();
    
    m_posX->setValue(0);
    m_posY->setValue(0);
    m_posZ->setValue(0);
    m_rotX->setValue(0);
    m_rotY->setValue(0);
    m_rotZ->setValue(0);

    clearSketchLayout();

    m_generalPanel->setExpanded(true);
    m_shapeInspectorPanel->setExpanded(false);
    m_transformPanel->setExpanded(false);
    m_sketchPanel->setExpanded(false);
}

void PropertyPanel::updateSelection(const Handle(AIS_Shape)& shape, const QString& name, ShapeType type)
{
    clear();

    if (shape.IsNull())
        return;

    m_currentShape = shape;
    m_nameEdit->setText(name);

    Quantity_Color currentColor;
    shape->Color(currentColor);
    QColor qColor(currentColor.Red() * 255, currentColor.Green() * 255, currentColor.Blue() * 255);
    QPixmap pixmap(56, 24);
    pixmap.fill(qColor);
    m_colorButton->setIcon(QIcon(pixmap));
    m_colorButton->setIconSize(QSize(56, 24));

    populateShapeInspector(shape->Shape());

    m_transformPanel->setExpanded(true);

    if (type != ShapeType::Model)
    {
        populateSketchProperties(shape, type);
        m_sketchPanel->setExpanded(true);
    }
}

void PropertyPanel::populateShapeInspector(const TopoDS_Shape& shape)
{
    m_shapeTree->clear();

    int solidCount = 0;
    int faceCount = 0;
    int wireCount = 0;
    int edgeCount = 0;
    int vertexCount = 0;

    TopExp_Explorer exp(shape, TopAbs_SOLID);
    for (; exp.More(); exp.Next()) solidCount++;

    exp.Init(shape, TopAbs_FACE);
    for (; exp.More(); exp.Next()) faceCount++;

    exp.Init(shape, TopAbs_WIRE);
    for (; exp.More(); exp.Next()) wireCount++;

    exp.Init(shape, TopAbs_EDGE);
    for (; exp.More(); exp.Next()) edgeCount++;

    exp.Init(shape, TopAbs_VERTEX);
    for (; exp.More(); exp.Next()) vertexCount++;

    QTreeWidgetItem* solidItem = new QTreeWidgetItem(m_shapeTree);
    solidItem->setText(0, "Solids");
    solidItem->setText(1, QString::number(solidCount));

    QTreeWidgetItem* faceItem = new QTreeWidgetItem(m_shapeTree);
    faceItem->setText(0, "Faces");
    faceItem->setText(1, QString::number(faceCount));

    QTreeWidgetItem* wireItem = new QTreeWidgetItem(m_shapeTree);
    wireItem->setText(0, "Wires");
    wireItem->setText(1, QString::number(wireCount));

    QTreeWidgetItem* edgeItem = new QTreeWidgetItem(m_shapeTree);
    edgeItem->setText(0, "Edges");
    edgeItem->setText(1, QString::number(edgeCount));

    QTreeWidgetItem* vertexItem = new QTreeWidgetItem(m_shapeTree);
    vertexItem->setText(0, "Vertices");
    vertexItem->setText(1, QString::number(vertexCount));
}

void PropertyPanel::clearSketchLayout()
{
    while (m_sketchLayout->count() > 0)
    {
        QLayoutItem* item = m_sketchLayout->takeAt(0);
        if (item->widget())
            delete item->widget();
        delete item;
    }
}

void PropertyPanel::populateSketchProperties(const Handle(AIS_Shape)& shape, ShapeType type)
{
    clearSketchLayout();

    QString labelStyle = 
        "QLabel { "
        "  font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif; "
        "  font-size: 13px; "
        "  color: #555555; "
        "  font-weight: 500; "
        "}";

    QString valueStyle = 
        "QLabel { "
        "  font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif; "
        "  font-size: 13px; "
        "  color: #666666; "
        "}";

    TopoDS_Shape topoShape = shape->Shape();

    switch (type)
    {
    case ShapeType::Point:
    {
        TopExp_Explorer exp(topoShape, TopAbs_VERTEX);
        if (exp.More())
        {
            TopoDS_Shape& shapeRef = const_cast<TopoDS_Shape&>(exp.Current());
            gp_Pnt pnt = BRep_Tool::Pnt(*(TopoDS_Vertex*)&shapeRef);

            QLabel* xName = new QLabel("X:");
                xName->setStyleSheet(labelStyle);
                QLabel* xLabel = new QLabel(QString::number(pnt.X(), 'f', 3));
                xLabel->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(xName, xLabel);

                QLabel* yName = new QLabel("Y:");
                yName->setStyleSheet(labelStyle);
                QLabel* yLabel = new QLabel(QString::number(pnt.Y(), 'f', 3));
                yLabel->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(yName, yLabel);

                QLabel* zName = new QLabel("Z:");
                zName->setStyleSheet(labelStyle);
                QLabel* zLabel = new QLabel(QString::number(pnt.Z(), 'f', 3));
                zLabel->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(zName, zLabel);
        }
        break;
    }

    case ShapeType::Line:
    {
        TopExp_Explorer exp(topoShape, TopAbs_EDGE);
        if (exp.More())
        {
            TopoDS_Shape& shapeRef = const_cast<TopoDS_Shape&>(exp.Current());
            TopoDS_Edge& edge = *(TopoDS_Edge*)&shapeRef;
            Standard_Real first, last;
            Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
            
            if (!curve.IsNull() && curve->IsKind(STANDARD_TYPE(Geom_Line)))
            {
                Handle(Geom_Line) line = Handle(Geom_Line)::DownCast(curve);
                gp_Pnt start = line->Position().Location();
                gp_Dir dir = line->Position().Direction();

                QLabel* sxName = new QLabel("Start X:");
                sxName->setStyleSheet(labelStyle);
                QLabel* startX = new QLabel(QString::number(start.X(), 'f', 3));
                startX->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(sxName, startX);

                QLabel* syName = new QLabel("Start Y:");
                syName->setStyleSheet(labelStyle);
                QLabel* startY = new QLabel(QString::number(start.Y(), 'f', 3));
                startY->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(syName, startY);

                QLabel* dxName = new QLabel("Direction X:");
                dxName->setStyleSheet(labelStyle);
                QLabel* dirX = new QLabel(QString::number(dir.X(), 'f', 3));
                dirX->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(dxName, dirX);

                QLabel* dyName = new QLabel("Direction Y:");
                dyName->setStyleSheet(labelStyle);
                QLabel* dirY = new QLabel(QString::number(dir.Y(), 'f', 3));
                dirY->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(dyName, dirY);
            }
        }
        break;
    }

    case ShapeType::Circle:
    {
        TopExp_Explorer exp(topoShape, TopAbs_EDGE);
        if (exp.More())
        {
            TopoDS_Shape& shapeRef = const_cast<TopoDS_Shape&>(exp.Current());
            TopoDS_Edge& edge = *(TopoDS_Edge*)&shapeRef;
            Standard_Real first, last;
            Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
            
            if (!curve.IsNull() && curve->IsKind(STANDARD_TYPE(Geom_Circle)))
            {
                Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(curve);
                gp_Pnt center = circle->Position().Location();
                double radius = circle->Radius();

                QLabel* cxName = new QLabel("Center X:");
                cxName->setStyleSheet(labelStyle);
                QLabel* centerX = new QLabel(QString::number(center.X(), 'f', 3));
                centerX->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(cxName, centerX);

                QLabel* cyName = new QLabel("Center Y:");
                cyName->setStyleSheet(labelStyle);
                QLabel* centerY = new QLabel(QString::number(center.Y(), 'f', 3));
                centerY->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(cyName, centerY);

                QLabel* rName = new QLabel("Radius:");
                rName->setStyleSheet(labelStyle);
                QLabel* radiusLabel = new QLabel(QString::number(radius, 'f', 3));
                radiusLabel->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(rName, radiusLabel);
            }
        }
        break;
    }

    case ShapeType::Arc:
    {
        TopExp_Explorer exp(topoShape, TopAbs_EDGE);
        if (exp.More())
        {
            TopoDS_Shape& shapeRef = const_cast<TopoDS_Shape&>(exp.Current());
            TopoDS_Edge& edge = *(TopoDS_Edge*)&shapeRef;
            Standard_Real first, last;
            Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
            
            if (!curve.IsNull() && curve->IsKind(STANDARD_TYPE(Geom_TrimmedCurve)))
            {
                Handle(Geom_TrimmedCurve) trimmedCurve = Handle(Geom_TrimmedCurve)::DownCast(curve);
                Handle(Geom_Curve) basisCurve = trimmedCurve->BasisCurve();
                
                if (!basisCurve.IsNull() && basisCurve->IsKind(STANDARD_TYPE(Geom_Circle)))
                {
                    Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(basisCurve);
                    gp_Pnt center = circle->Position().Location();
                    double radius = circle->Radius();
                    double startParam = trimmedCurve->FirstParameter();
                    double endParam = trimmedCurve->LastParameter();

                    QLabel* cxName = new QLabel("Center X:");
                cxName->setStyleSheet(labelStyle);
                QLabel* centerX = new QLabel(QString::number(center.X(), 'f', 3));
                centerX->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(cxName, centerX);

                QLabel* cyName = new QLabel("Center Y:");
                cyName->setStyleSheet(labelStyle);
                QLabel* centerY = new QLabel(QString::number(center.Y(), 'f', 3));
                centerY->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(cyName, centerY);

                QLabel* rName = new QLabel("Radius:");
                rName->setStyleSheet(labelStyle);
                QLabel* radiusLabel = new QLabel(QString::number(radius, 'f', 3));
                radiusLabel->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(rName, radiusLabel);

                QLabel* saName = new QLabel("Start Angle:");
                saName->setStyleSheet(labelStyle);
                QLabel* startAngle = new QLabel(QString::number(startParam * 180.0 / M_PI, 'f', 1) + " deg");
                startAngle->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(saName, startAngle);

                QLabel* eaName = new QLabel("End Angle:");
                eaName->setStyleSheet(labelStyle);
                QLabel* endAngle = new QLabel(QString::number(endParam * 180.0 / M_PI, 'f', 1) + " deg");
                endAngle->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(eaName, endAngle);
                }
            }
        }
        break;
    }

    case ShapeType::Polyline:
    {
        TopExp_Explorer exp(topoShape, TopAbs_VERTEX);
        int vCount = 0;
        while (exp.More())
        {
            TopoDS_Shape& shapeRef = const_cast<TopoDS_Shape&>(exp.Current());
            gp_Pnt pnt = BRep_Tool::Pnt(*(TopoDS_Vertex*)&shapeRef);
            QLabel* ptName = new QLabel(QString("Pt%1:").arg(++vCount));
            ptName->setStyleSheet(labelStyle);
            QLabel* ptLabel = new QLabel(QString("(%1, %2, %3)")
                .arg(QString::number(pnt.X(), 'f', 3))
                .arg(QString::number(pnt.Y(), 'f', 3))
                .arg(QString::number(pnt.Z(), 'f', 3)));
            ptLabel->setStyleSheet(valueStyle);
            m_sketchLayout->addRow(ptName, ptLabel);
            exp.Next();
        }
        QLabel* segName = new QLabel("Segments:");
        segName->setStyleSheet(labelStyle);
        QLabel* segLabel = new QLabel(QString::number(qMax(0, vCount - 1)));
        segLabel->setStyleSheet(valueStyle);
        m_sketchLayout->addRow(segName, segLabel);
        break;
    }

    case ShapeType::Spline:
    {
        TopExp_Explorer exp(topoShape, TopAbs_EDGE);
        if (exp.More())
        {
            TopoDS_Shape& shapeRef = const_cast<TopoDS_Shape&>(exp.Current());
            TopoDS_Edge& edge = *(TopoDS_Edge*)&shapeRef;
            Standard_Real first, last;
            Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
            
            if (!curve.IsNull() && curve->IsKind(STANDARD_TYPE(Geom_BSplineCurve)))
            {
                Handle(Geom_BSplineCurve) bspline = Handle(Geom_BSplineCurve)::DownCast(curve);
                int nbPoles = bspline->NbPoles();
                int nbKnots = bspline->NbKnots();
                int degree = bspline->Degree();

                QLabel* degName = new QLabel("Degree:");
                degName->setStyleSheet(labelStyle);
                QLabel* degLabel = new QLabel(QString::number(degree));
                degLabel->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(degName, degLabel);

                QLabel* cpName = new QLabel("Control Points:");
                cpName->setStyleSheet(labelStyle);
                QLabel* cpLabel = new QLabel(QString::number(nbPoles));
                cpLabel->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(cpName, cpLabel);

                // Show control point positions
                for (int i = 1; i <= nbPoles; ++i)
                {
                    gp_Pnt cp = bspline->Pole(i);
                    QLabel* cptName = new QLabel(QString("CP%1:").arg(i));
                    cptName->setStyleSheet(labelStyle);
                    QLabel* cptLabel = new QLabel(QString("(%1, %2, %3)")
                        .arg(QString::number(cp.X(), 'f', 3))
                        .arg(QString::number(cp.Y(), 'f', 3))
                        .arg(QString::number(cp.Z(), 'f', 3)));
                    cptLabel->setStyleSheet(valueStyle);
                    m_sketchLayout->addRow(cptName, cptLabel);
                }
            }
        }
        break;
    }

    case ShapeType::Ellipse:
    {
        TopExp_Explorer exp(topoShape, TopAbs_EDGE);
        if (exp.More())
        {
            TopoDS_Shape& shapeRef = const_cast<TopoDS_Shape&>(exp.Current());
            TopoDS_Edge& edge = *(TopoDS_Edge*)&shapeRef;
            Standard_Real first, last;
            Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
            
            if (!curve.IsNull() && curve->IsKind(STANDARD_TYPE(Geom_Ellipse)))
            {
                Handle(Geom_Ellipse) ellipse = Handle(Geom_Ellipse)::DownCast(curve);
                gp_Elips elips = ellipse->Elips();
                gp_Pnt center = elips.Location();
                double majorR = elips.MajorRadius();
                double minorR = elips.MinorRadius();

                QLabel* cxName = new QLabel("Center X:");
                cxName->setStyleSheet(labelStyle);
                QLabel* centerX = new QLabel(QString::number(center.X(), 'f', 3));
                centerX->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(cxName, centerX);

                QLabel* cyName = new QLabel("Center Y:");
                cyName->setStyleSheet(labelStyle);
                QLabel* centerY = new QLabel(QString::number(center.Y(), 'f', 3));
                centerY->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(cyName, centerY);

                QLabel* maName = new QLabel("Major Radius:");
                maName->setStyleSheet(labelStyle);
                QLabel* majorLabel = new QLabel(QString::number(majorR, 'f', 3));
                majorLabel->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(maName, majorLabel);

                QLabel* miName = new QLabel("Minor Radius:");
                miName->setStyleSheet(labelStyle);
                QLabel* minorLabel = new QLabel(QString::number(minorR, 'f', 3));
                minorLabel->setStyleSheet(valueStyle);
                m_sketchLayout->addRow(miName, minorLabel);
            }
        }
        break;
    }

    case ShapeType::Rectangle:
    {
        TopExp_Explorer exp(topoShape, TopAbs_VERTEX);
        int vCount = 0;
        gp_Pnt corners[4];
        while (exp.More() && vCount < 4)
        {
            TopoDS_Shape& shapeRef = const_cast<TopoDS_Shape&>(exp.Current());
            gp_Pnt pnt = BRep_Tool::Pnt(*(TopoDS_Vertex*)&shapeRef);
            corners[vCount++] = pnt;
            exp.Next();
        }
        for (int i = 0; i < vCount; ++i)
        {
            QLabel* corName = new QLabel(QString("Corner %1:").arg(i + 1));
            corName->setStyleSheet(labelStyle);
            QLabel* corLabel = new QLabel(QString("(%1, %2, %3)")
                .arg(QString::number(corners[i].X(), 'f', 3))
                .arg(QString::number(corners[i].Y(), 'f', 3))
                .arg(QString::number(corners[i].Z(), 'f', 3)));
            corLabel->setStyleSheet(valueStyle);
            m_sketchLayout->addRow(corName, corLabel);
        }
        if (vCount >= 2)
        {
            double w = fabs(corners[0].X() - corners[1].X());
            double h = fabs(corners[0].Y() - corners[2].Y());

            QLabel* wName = new QLabel("Width:");
            wName->setStyleSheet(labelStyle);
            QLabel* wLabel = new QLabel(QString::number(w, 'f', 3));
            wLabel->setStyleSheet(valueStyle);
            m_sketchLayout->addRow(wName, wLabel);

            QLabel* hName = new QLabel("Height:");
            hName->setStyleSheet(labelStyle);
            QLabel* hLabel = new QLabel(QString::number(h, 'f', 3));
            hLabel->setStyleSheet(valueStyle);
            m_sketchLayout->addRow(hName, hLabel);
        }
        break;
    }

    default:
        break;
    }
}

void PropertyPanel::onNameChanged(const QString& text)
{
    emit nameChanged(text);
}

void PropertyPanel::onTransformChanged()
{
}

void PropertyPanel::onApplyTransform()
{
    emit applyTransform(m_posX->value(), m_posY->value(), m_posZ->value(),
                        m_rotX->value(), m_rotY->value(), m_rotZ->value());
}

void PropertyPanel::onColorClicked()
{
    if (m_currentShape.IsNull())
        return;

    Quantity_Color currentColor;
    m_currentShape->Color(currentColor);

    QColor qColor(currentColor.Red() * 255, currentColor.Green() * 255, currentColor.Blue() * 255);
    QColor selectedColor = QColorDialog::getColor(qColor, this, "Select Color");
    
    if (selectedColor.isValid()) {
        QPixmap pixmap(56, 24);
        pixmap.fill(selectedColor);
        m_colorButton->setIcon(QIcon(pixmap));
        m_colorButton->setIconSize(QSize(56, 24));
        
        Quantity_Color newColor(selectedColor.redF(), selectedColor.greenF(), selectedColor.blueF(), Quantity_TOC_RGB);
        emit colorChanged(m_currentShape, newColor);
    }
}
