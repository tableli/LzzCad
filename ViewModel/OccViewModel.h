#pragma once

#include <QObject>
#include <QString>
#include <TopoDS_Shape.hxx>
#include <QMetaType>

Q_DECLARE_METATYPE(TopoDS_Shape)

#include "../Model/GeometryModel.h"

class GeometryModel;

class OccViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(bool isShaded READ isShaded NOTIFY isShadedChanged)
    
public:
    explicit OccViewModel(GeometryModel* model, QObject* parent = nullptr);
    
    QString statusText() const { return m_statusText; }
    bool isShaded() const { return m_isShaded; }
    
    Q_INVOKABLE void fitAll();
    Q_INVOKABLE void rotate();
    Q_INVOKABLE void pan();
    Q_INVOKABLE void zoom();
    
    Q_INVOKABLE void viewTop();
    Q_INVOKABLE void viewBottom();
    Q_INVOKABLE void viewLeft();
    Q_INVOKABLE void viewRight();
    Q_INVOKABLE void viewFront();
    Q_INVOKABLE void viewBack();
    
    Q_INVOKABLE void toggleShaded();
    Q_INVOKABLE void setShaded(bool shaded);
    
    Q_INVOKABLE void addBox(double x, double y, double z);
    Q_INVOKABLE void clearAllShapes();
    
    Q_INVOKABLE void addPoint(const Handle(AIS_Shape)& shape);
    Q_INVOKABLE void addLine(const Handle(AIS_Shape)& shape);
    Q_INVOKABLE void addCircle(const Handle(AIS_Shape)& shape);
    Q_INVOKABLE void addArc(const Handle(AIS_Shape)& shape);
    Q_INVOKABLE void addShape(const Handle(AIS_Shape)& shape);
    Q_INVOKABLE void addShapeWithType(const Handle(AIS_Shape)& shape, ShapeType type);
    Q_INVOKABLE void addModel(const Handle(AIS_Shape)& shape, const QString& name = "");
    
    Q_INVOKABLE void openFile(const QString& fileName);
    Q_INVOKABLE void saveFile(const QString& fileName);
    
signals:
    void statusTextChanged();
    void isShadedChanged();
    
    void requestFitAll();
    void requestRotate();
    void requestPan();
    void requestZoom();
    
    void requestViewTop();
    void requestViewBottom();
    void requestViewLeft();
    void requestViewRight();
    void requestViewFront();
    void requestViewBack();
    
    void requestSetShaded(bool shaded);
    
    void requestAddShape(const Handle(AIS_Shape)& shape);
    void requestClearAll();
    
    void fileOpened(bool success, const QString& message);
    void fileSaved(bool success, const QString& message);
    
    void shapeAddedToModel(const QString& name);
    
private:
    void updateStatus(const QString& status);
    
    GeometryModel* m_model = nullptr;
    QString m_statusText = "Ready";
    bool m_isShaded = true;
};
