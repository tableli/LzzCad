#pragma once

#include <QWidget>
#include <QMenu>
#include <QRubberBand>

#include <AIS_InteractiveContext.hxx>
#include <TopoDS_Shape.hxx>
#include <AIS_Shape.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <AIS_Manipulator.hxx>
#include <AIS_ViewCube.hxx>
#include <Prs3d_Drawer.hxx>
#include <gp_Trsf.hxx>
#include <gp_Ax1.hxx>
#include <gp_Vec.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <set>
#include<qdebug.h>
#include "Model/GeometryModel.h"

class OccViewModel;
class CommandManager;

class OccView : public QWidget
{
    Q_OBJECT

public:
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
        CurAction3d_Sketch_DrawArc,
        CurAction3d_Primitive_CreateBox,
        CurAction3d_Primitive_CreateSphere,
        CurAction3d_Primitive_CreateCylinder,
        CurAction3d_Primitive_CreateCone,
        CurAction3d_Feature_Extrude,
        CurAction3d_Feature_Revolve,
        CurAction3d_Feature_Sweep
    };

public:
    OccView(QWidget* parent = nullptr);
    ~OccView();

    void setViewModel(OccViewModel* viewModel);
    void setCommandManager(CommandManager* mgr);
    
    const Handle(AIS_InteractiveContext)& getContext() const;
    Handle(V3d_View) getView() const { return myView; }

    Handle(AIS_Shape) SelectedShape() const { return mySelectedShape; }
    void setSelectedShape(const Handle(AIS_Shape)& shape) { mySelectedShape = shape; }
    
    void displayShape(const Handle(AIS_Shape)& shape);
    void clearAllShapes();
    
    void hideShape(const Handle(AIS_Shape)& shape);
    void showShape(const Handle(AIS_Shape)& shape);
    bool isShapeVisible(const Handle(AIS_Shape)& shape);
    
    void transformShape(const Handle(AIS_Shape)& shape, double dx, double dy, double dz, double rx, double ry, double rz);
    void updateViewer();

    // Sketch mode control
    void startSketchPointMode();
    void startSketchLineMode();
    void startSketchCircleMode();
    void startSketchArcMode();
    void stopSketchMode();

    // Primitive mode control
    void startPrimitiveBoxMode();
    void startPrimitiveSphereMode();
    void startPrimitiveCylinderMode();
    void startPrimitiveConeMode();
    void stopPrimitiveMode();

    // Feature mode control
    void startExtrudeMode();
    void stopExtrudeMode();
    void startRevolveMode();
    void stopRevolveMode();
    void startSweepMode();
    void stopSweepMode();

protected:
    void performExtrude(const Handle(AIS_Shape)& sketchShape);
    void performRevolve(const Handle(AIS_Shape)& sketchShape);
    void performSweep(const Handle(AIS_Shape)& profileShape, const Handle(AIS_Shape)& pathShape);

signals:
    void selectionChanged(void);
    void mouseMoved(const QPoint& pos);
    void shapeCreated(const Handle(AIS_Shape)& shape, ShapeType type);

public slots:
    void pan(void);
    void fitAll(void);
    void reset(void);
    void zoom(void);
    void rotate(void);
    void top(void);
    void bottom(void);
    void left(void);
    void right(void);
    void front(void);
    void back(void);
    void shaded(void);
    void wireframe(void);
    void cancelSketch(void);
    
    void drawPoint(double x, double y);
    void drawLine(double x1, double y1, double x2, double y2);
    void drawPolyline(const QVector<QPair<double, double>>& points);
    void drawPolygon(const QVector<QPair<double, double>>& points);
    void drawCircle(double cx, double cy, double radius);
    void drawArc(double cx, double cy, double radius, double startAngle, double sweepAngle);
    void eraseSelected(void);
    QList<Handle(AIS_InteractiveObject)> getSelectedObjects(void);
    
    void onViewModelShadedChanged(bool shaded);
    
    void selectShape(const Handle(AIS_Shape)& shape);

protected:
    virtual QPaintEngine* paintEngine() const override;
    virtual void paintEvent(QPaintEvent* theEvent) override;
    virtual void resizeEvent(QResizeEvent* theEvent) override;
    virtual void showEvent(QShowEvent* theEvent) override;
    virtual void mousePressEvent(QMouseEvent* theEvent) override;
    virtual void mouseReleaseEvent(QMouseEvent* theEvent) override;
    virtual void mouseMoveEvent(QMouseEvent* theEvent) override;
    virtual void wheelEvent(QWheelEvent* theEvent) override;
    virtual void keyPressEvent(QKeyEvent* theEvent) override;

    virtual void onLButtonDown(const int theFlags, const QPoint thePoint);
    virtual void onMButtonDown(const int theFlags, const QPoint thePoint);
    virtual void onRButtonDown(const int theFlags, const QPoint thePoint);
    virtual void onMouseWheel(const int theFlags, const int theDelta, const QPoint thePoint);
    virtual void onLButtonUp(const int theFlags, const QPoint thePoint);
    virtual void onMButtonUp(const int theFlags, const QPoint thePoint);
    virtual void onRButtonUp(const int theFlags, const QPoint thePoint);
    virtual void onMouseMove(const int theFlags, const QPoint thePoint);

protected:
    void init(void);
    void popup(const int x, const int y);
    void dragEvent(const int x, const int y);
    void inputEvent(const int x, const int y);
    void moveEvent(const int x, const int y);
    void multiMoveEvent(const int x, const int y);
    void multiDragEvent(const int x, const int y);
    void multiInputEvent(const int x, const int y);
    void drawRubberBand(const int minX, const int minY, const int maxX, const int maxY);
    void panByMiddleButton(const QPoint& thePoint);

    // Sketch drawing helpers
    gp_Pnt convertScreenToWorld(const QPoint& screenPoint);
    void updateSketchPreview();
    void finishSketchShape();
    void resetSketchState();

    // Primitive creation helpers
    void handlePrimitiveCreation(QMouseEvent* theEvent);
    void handlePrimitiveMouseMove(QMouseEvent* theEvent);
    void handleBoxCreation(const gp_Pnt& clickedPoint);
    void updateBoxPreview(const gp_Pnt& mousePoint);
    void handleSphereCreation(const gp_Pnt& clickedPoint);
    void updateSpherePreview(const gp_Pnt& mousePoint);
    void handleCylinderCreation(const gp_Pnt& clickedPoint);
    void updateCylinderPreview(const gp_Pnt& mousePoint);
    void handleConeCreation(const gp_Pnt& clickedPoint);
    void updateConePreview(const gp_Pnt& mousePoint);
    void createPrimitiveShape(const TopoDS_Shape& shape, ShapeType type);

    // Sketch point counter
    int m_sketchClickCount;

protected:
    Handle(V3d_Viewer) myViewer;
    Handle(AIS_InteractiveContext) myContext;
    Handle(V3d_View) myView;
    Handle(AIS_Manipulator) myManipulator;
    Handle(AIS_Shape) mySelectedShape;

    bool isManipulatorActive = false;
    bool isMousePressed = false;

    Standard_Integer myXmin;
    Standard_Integer myYmin;
    Standard_Integer myXmax;
    Standard_Integer myYmax;

    CurrentAction3d myCurrentMode;
    Standard_Boolean myDegenerateModeIsOn;
    QRubberBand* myRectBand;

    std::set<Handle(AIS_InteractiveObject)> hiddenObjects;
    
    bool m_isInitialized;
    
    // Sketch drawing state
    Handle(AIS_Shape) m_previewShape;
    Handle(AIS_Shape) m_arcCenterPoint;
    gp_Pnt m_sketchPoint1;
    gp_Pnt m_sketchPoint2;
    gp_Pnt m_sketchPoint3;
    bool m_sketchReverseArc;

    // Primitive creation state
    enum PrimitivePhase {
        PrimitivePhase_Start,
        PrimitivePhase_Base,
        PrimitivePhase_Height,
        PrimitivePhase_Radius2
    };
    PrimitivePhase m_primitivePhase;
    gp_Pnt m_primitivePoint1;
    gp_Pnt m_primitivePoint2;
    double m_primitiveRadius1;
    double m_primitiveRadius2;
    double m_primitiveHeight;

    // Sweep mode state
    Handle(AIS_Shape) m_sweepProfile;

private:
    OccViewModel* m_viewModel = nullptr;
    CommandManager* m_commandManager = nullptr;
};
