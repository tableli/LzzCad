#include "OccViewModel.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include <QFileInfo>

static int registerTopoDSShapeType = qRegisterMetaType<TopoDS_Shape>("TopoDS_Shape");

OccViewModel::OccViewModel(GeometryModel* model, QObject* parent)
    : QObject(parent), m_model(model) {
    updateStatus("ViewModel initialized");
}

void OccViewModel::updateStatus(const QString& status) {
    m_statusText = status;
    emit statusTextChanged();
}

void OccViewModel::fitAll() {
    updateStatus("Fit all objects");
    emit requestFitAll();
}

void OccViewModel::rotate() {
    updateStatus("Rotation mode");
    emit requestRotate();
}

void OccViewModel::pan() {
    updateStatus("Pan mode");
    emit requestPan();
}

void OccViewModel::zoom() {
    updateStatus("Zoom mode");
    emit requestZoom();
}

void OccViewModel::viewTop() {
    updateStatus("Top view");
    emit requestViewTop();
}

void OccViewModel::viewBottom() {
    updateStatus("Bottom view");
    emit requestViewBottom();
}

void OccViewModel::viewLeft() {
    updateStatus("Left view");
    emit requestViewLeft();
}

void OccViewModel::viewRight() {
    updateStatus("Right view");
    emit requestViewRight();
}

void OccViewModel::viewFront() {
    updateStatus("Front view");
    emit requestViewFront();
}

void OccViewModel::viewBack() {
    updateStatus("Back view");
    emit requestViewBack();
}

void OccViewModel::toggleShaded() {
    setShaded(!m_isShaded);
}

void OccViewModel::setShaded(bool shaded) {
    m_isShaded = shaded;
    emit isShadedChanged();
    updateStatus(m_isShaded ? "Shaded mode" : "Wireframe mode");
    emit requestSetShaded(shaded);
}

void OccViewModel::addBox(double x, double y, double z) {
    TopoDS_Shape box = BRepPrimAPI_MakeBox(x, y, z).Shape();
    m_model->addShape(box, ShapeType::Model);
    QString name = m_model->getShapeNames().back();
    updateStatus(QString("Added box: %1x%2x%3").arg(x).arg(y).arg(z));
    const auto& shapes = m_model->getShapes();
    if (!shapes.empty()) {
        emit requestAddShape(shapes.back());
    }
    emit shapeAddedToModel(name);
}

void OccViewModel::clearAllShapes() {
    m_model->clearAll();
    updateStatus("Cleared all shapes");
    emit requestClearAll();
}

void OccViewModel::addPoint(const Handle(AIS_Shape)& shape) {
    m_model->addShape(shape, ShapeType::Point);
    QString name = m_model->getShapeNames().back();
    updateStatus("Added point");
    emit shapeAddedToModel(name);
}

void OccViewModel::addLine(const Handle(AIS_Shape)& shape) {
    m_model->addShape(shape, ShapeType::Line);
    QString name = m_model->getShapeNames().back();
    updateStatus("Added line");
    emit shapeAddedToModel(name);
}

void OccViewModel::addCircle(const Handle(AIS_Shape)& shape) {
    m_model->addShape(shape, ShapeType::Circle);
    QString name = m_model->getShapeNames().back();
    updateStatus("Added circle");
    emit shapeAddedToModel(name);
}

void OccViewModel::addArc(const Handle(AIS_Shape)& shape) {
    m_model->addShape(shape, ShapeType::Arc);
    QString name = m_model->getShapeNames().back();
    updateStatus("Added arc");
    emit shapeAddedToModel(name);
}

void OccViewModel::addShape(const Handle(AIS_Shape)& shape) {
    m_model->addShape(shape);
    QString name = m_model->getShapeNames().back();
    updateStatus("Added shape");
    emit shapeAddedToModel(name);
}

void OccViewModel::addShapeWithType(const Handle(AIS_Shape)& shape, ShapeType type) {
    m_model->addShape(shape, type);
    QString name = m_model->getShapeNames().back();
    updateStatus("Added shape");
    emit shapeAddedToModel(name);
}

void OccViewModel::addModel(const Handle(AIS_Shape)& shape, const QString& name) {
    m_model->addShape(shape, ShapeType::Model, name);
    QString shapeName = m_model->getShapeNames().back();
    updateStatus("Added model");
    emit shapeAddedToModel(shapeName);
}

void OccViewModel::openFile(const QString& fileName) {
    std::string errorMessage;
    bool success = false;
    
    QFileInfo fileInfo(fileName);
    QString extension = fileInfo.suffix().toLower();
    QString modelName = fileInfo.baseName();
    
    if (extension == "step" || extension == "stp") {
        success = m_model->readStepFile(fileName.toStdString(), errorMessage);
    } else if (extension == "brep" || extension == "brp") {
        success = m_model->readBrepFile(fileName.toStdString(), errorMessage);
    } else if (extension == "iges" || extension == "igs") {
        success = m_model->readIgesFile(fileName.toStdString(), errorMessage);
    } else {
        errorMessage = "Unsupported file format";
    }
    
    if (success) {
        updateStatus("File opened: " + fileName);
        
        const auto& shapes = m_model->getShapes();
        const auto& names = m_model->getShapeNames();
        if (!shapes.empty()) {
            emit requestAddShape(shapes.back());
            emit shapeAddedToModel(names.back());
        }
        
        emit fileOpened(true, "File opened successfully");
    } else {
        emit fileOpened(false, QString::fromStdString(errorMessage));
    }
}

void OccViewModel::saveFile(const QString& fileName) {
    std::string errorMessage;
    bool success = false;
    
    QFileInfo fileInfo(fileName);
    QString extension = fileInfo.suffix().toLower();
    
    if (extension == "step" || extension == "stp") {
        success = m_model->writeStepFile(fileName.toStdString(), errorMessage);
    } else if (extension == "brep" || extension == "brp") {
        success = m_model->writeBrepFile(fileName.toStdString(), errorMessage);
    } else if (extension == "iges" || extension == "igs") {
        success = m_model->writeIgesFile(fileName.toStdString(), errorMessage);
    } else {
        errorMessage = "Unsupported file format";
    }
    
    if (success) {
        updateStatus("File saved: " + fileName);
        emit fileSaved(true, "File saved successfully");
    } else {
        emit fileSaved(false, QString::fromStdString(errorMessage));
    }
}
