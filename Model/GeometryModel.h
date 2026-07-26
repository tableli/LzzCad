#pragma once

#include <TopoDS_Shape.hxx>
#include <AIS_Shape.hxx>
#include <AIS_InteractiveObject.hxx>
#include <vector>
#include <string>
#include <QString>

enum class ShapeType {
    Unknown,
    Point,
    Line,
    Circle,
    Arc,
    Ellipse,
    Spline,
    Polyline,
    Rectangle,
    Box,
    Sphere,
    Cylinder,
    Cone,
    Extrude,
    Revolve,
    Sweep,
    Model
};

class GeometryModel {

public:
    GeometryModel() {}
    
    void addShape(const Handle(AIS_Shape)& aisShape, ShapeType type = ShapeType::Unknown, const QString& name = "") {
        m_shapes.push_back(aisShape);
        m_shapeTypes.push_back(type);
        m_shapeNames.push_back(name.isEmpty() ? generateShapeName(type) : name);
    }
    
    void addShape(const TopoDS_Shape& shape, ShapeType type = ShapeType::Unknown, const QString& name = "") {
        Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
        m_shapes.push_back(aisShape);
        m_shapeTypes.push_back(type);
        m_shapeNames.push_back(name.isEmpty() ? generateShapeName(type) : name);
    }
    
    void removeShape(const Handle(AIS_Shape)& aisShape) {
        auto it = std::find(m_shapes.begin(), m_shapes.end(), aisShape);
        if (it != m_shapes.end()) {
            size_t index = std::distance(m_shapes.begin(), it);
            m_shapes.erase(it);
            m_shapeTypes.erase(m_shapeTypes.begin() + index);
            m_shapeNames.erase(m_shapeNames.begin() + index);
        }
    }
    
    void removeShapeByName(const QString& name) {
        auto it = std::find(m_shapeNames.begin(), m_shapeNames.end(), name);
        if (it != m_shapeNames.end()) {
            size_t index = std::distance(m_shapeNames.begin(), it);
            m_shapes.erase(m_shapes.begin() + index);
            m_shapeTypes.erase(m_shapeTypes.begin() + index);
            m_shapeNames.erase(it);
        }
    }
    
    void removeShapeByInteractiveObject(const Handle(AIS_InteractiveObject)& obj) {
        Handle(AIS_Shape) aisShape = Handle(AIS_Shape)::DownCast(obj);
        if (!aisShape.IsNull()) {
            removeShape(aisShape);
        }
    }
    
    const std::vector<Handle(AIS_Shape)>& getShapes() const {
        return m_shapes;
    }
    
    const std::vector<QString>& getShapeNames() const {
        return m_shapeNames;
    }
    
    const std::vector<ShapeType>& getShapeTypes() const {
        return m_shapeTypes;
    }
    
    QString getShapeName(const Handle(AIS_Shape)& shape) const {
        auto it = std::find(m_shapes.begin(), m_shapes.end(), shape);
        if (it != m_shapes.end()) {
            size_t index = std::distance(m_shapes.begin(), it);
            return m_shapeNames[index];
        }
        return "";
    }
    
    ShapeType getShapeType(const Handle(AIS_Shape)& shape) const {
        auto it = std::find(m_shapes.begin(), m_shapes.end(), shape);
        if (it != m_shapes.end()) {
            size_t index = std::distance(m_shapes.begin(), it);
            return m_shapeTypes[index];
        }
        return ShapeType::Unknown;
    }
    
    size_t shapeCount() const {
        return m_shapes.size();
    }
    
    void clearAll() {
        m_shapes.clear();
        m_shapeTypes.clear();
        m_shapeNames.clear();
    }
    
    bool readStepFile(const std::string& fileName, std::string& errorMessage);
    bool readBrepFile(const std::string& fileName, std::string& errorMessage);
    bool readIgesFile(const std::string& fileName, std::string& errorMessage);
    
    bool writeStepFile(const std::string& fileName, std::string& errorMessage);
    bool writeBrepFile(const std::string& fileName, std::string& errorMessage);
    bool writeIgesFile(const std::string& fileName, std::string& errorMessage);

private:
    QString generateShapeName(ShapeType type) {
        static int pointCount = 0;
        static int lineCount = 0;
        static int circleCount = 0;
        static int arcCount = 0;
        static int boxCount = 0;
        static int sphereCount = 0;
        static int cylinderCount = 0;
        static int coneCount = 0;
        static int extrudeCount = 0;
        static int revolveCount = 0;
        static int sweepCount = 0;
        static int modelCount = 0;
        static int ellipseCount = 0;
        static int polylineCount = 0;
        static int rectangleCount = 0;
        static int splineCount = 0;
        
        switch (type) {
            case ShapeType::Point: return QString("Point_%1").arg(++pointCount);
            case ShapeType::Line: return QString("Line_%1").arg(++lineCount);
            case ShapeType::Circle: return QString("Circle_%1").arg(++circleCount);
            case ShapeType::Arc: return QString("Arc_%1").arg(++arcCount);
            case ShapeType::Box: return QString("Box_%1").arg(++boxCount);
            case ShapeType::Sphere: return QString("Sphere_%1").arg(++sphereCount);
            case ShapeType::Cylinder: return QString("Cylinder_%1").arg(++cylinderCount);
            case ShapeType::Cone: return QString("Cone_%1").arg(++coneCount);
            case ShapeType::Extrude: return QString("Extrude_%1").arg(++extrudeCount);
            case ShapeType::Revolve: return QString("Revolve_%1").arg(++revolveCount);
            case ShapeType::Sweep: return QString("Sweep_%1").arg(++sweepCount);
            case ShapeType::Model: return QString("Model_%1").arg(++modelCount);
            case ShapeType::Ellipse: return QString("Ellipse_%1").arg(++ellipseCount);
            case ShapeType::Polyline: return QString("Polyline_%1").arg(++polylineCount);
            case ShapeType::Rectangle: return QString("Rectangle_%1").arg(++rectangleCount);
            case ShapeType::Spline: return QString("Spline_%1").arg(++splineCount);
            default: return QString("Shape_%1").arg(m_shapes.size());
        }
    }
    
    std::vector<Handle(AIS_Shape)> m_shapes;
    std::vector<QString> m_shapeNames;
    std::vector<ShapeType> m_shapeTypes;
};
