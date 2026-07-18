#include "view.h"
#include "ViewModel/OccViewModel.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QStyleFactory>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QFont>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QDialog>
#include <QComboBox>

#include <OpenGl_GraphicDriver.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <TopAbs.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>
#include <V3d_DirectionalLight.hxx>
#include <V3d_AmbientLight.hxx>
#include <V3d_TypeOfOrientation.hxx>
#include <V3d_TypeOfVisualization.hxx>
#include <Quantity_Color.hxx>
#include <Graphic3d_TransformPers.hxx>
#include <Graphic3d_Vec2.hxx>
#include <Graphic3d_TypeOfShadingModel.hxx>
#include <Aspect_GradientFillMethod.hxx>
#include <Aspect_GridType.hxx>
#include <Aspect_GridDrawMode.hxx>
#include <Aspect_TypeOfTriedronPosition.hxx>
#include <AIS_DisplayMode.hxx>
#include <AIS_ViewCube.hxx>
#include <Aspect_Grid.hxx>
#include <Prs3d_PointAspect.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_TypeOfHighlight.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Graphic3d_ArrayOfPoints.hxx>
#include <Aspect_Handle.hxx>

#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax2.hxx>
#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <Geom_Circle.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepOffsetAPI_MakePipe.hxx>
#include <ElCLib.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <GeomAPI_IntCS.hxx>
#include "E:\MyGithub\LzzCad\Command\Command.h"

#ifdef _WIN32
  #include <WNT_Window.hxx>
#elif defined(__APPLE__) && !defined(MACOSX_USE_GLX)
  #include <Cocoa_Window.hxx>
#else
  #undef Bool
  #undef None
  #undef KeyPress
  #undef KeyRelease
  #undef FocusOut
  #undef FontChange
  #include <Xw_Window.hxx>
#endif
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>

static Handle(Graphic3d_GraphicDriver)& GetGraphicDriver()
{
    static Handle(Graphic3d_GraphicDriver) aGraphicDriver;
    return aGraphicDriver;
}

OccView::OccView(QWidget* parent)
    : QWidget(parent),
    myXmin(0),
    myYmin(0),
    myXmax(0),
    myYmax(0),
    myCurrentMode(CurAction3d_DynamicRotation),
    myDegenerateModeIsOn(Standard_True),
    myRectBand(nullptr),
    m_viewModel(nullptr),
    m_sketchClickCount(0),
    m_sketchReverseArc(false),
    m_isInitialized(false)
{
    setBackgroundRole(QPalette::NoRole);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setMouseTracking(true);
}

OccView::~OccView()
{
    if (myRectBand) {
        delete myRectBand;
    }
}

void OccView::setCommandManager(CommandManager* mgr)
{
    m_commandManager = mgr;
}

void OccView::setViewModel(OccViewModel* viewModel)
{
    m_viewModel = viewModel;
}

const Handle(AIS_InteractiveContext)& OccView::getContext() const
{
    return myContext;
}

void OccView::displayShape(const Handle(AIS_Shape)& aisShape)
{
    if (myContext.IsNull() || aisShape.IsNull()) return;

    aisShape->SetDisplayMode(AIS_Shaded);
    myContext->Display(aisShape, Standard_True);
    myContext->ClearSelected(Standard_False);
    myContext->UpdateCurrentViewer();
    myView->Redraw();
}

void OccView::updateViewer()
{
    if (myContext.IsNull()) return;
    myContext->UpdateCurrentViewer();
    myView->Redraw();
}

void OccView::clearAllShapes()
{
    if (myContext.IsNull()) return;

    myContext->EraseAll(Standard_True);
    myView->Redraw();
}

void OccView::hideShape(const Handle(AIS_Shape)& shape)
{
    if (myContext.IsNull() || shape.IsNull()) return;
    
    if (myContext->IsDisplayed(shape))
    {
        myContext->Erase(shape, Standard_False);
        myContext->UpdateCurrentViewer();
        myView->Redraw();
    }
}

void OccView::showShape(const Handle(AIS_Shape)& shape)
{
    if (myContext.IsNull() || shape.IsNull()) return;
    
    if (!myContext->IsDisplayed(shape))
    {
        myContext->Display(shape, Standard_True);
        myContext->UpdateCurrentViewer();
        myView->Redraw();
    }
}

bool OccView::isShapeVisible(const Handle(AIS_Shape)& shape)
{
    if (myContext.IsNull() || shape.IsNull()) return false;
    
    return myContext->IsDisplayed(shape);
}

void OccView::transformShape(const Handle(AIS_Shape)& shape, double dx, double dy, double dz, double rx, double ry, double rz)
{
    if (myContext.IsNull() || shape.IsNull()) return;

    TopoDS_Shape topoShape = shape->Shape();

    gp_Trsf trsf;

    if (rx != 0.0) {
        gp_Trsf rotX;
        gp_Ax1 ax1(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
        rotX.SetRotation(ax1, rx * M_PI / 180.0);
        trsf.Multiply(rotX);
    }

    if (ry != 0.0) {
        gp_Trsf rotY;
        gp_Ax1 ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0));
        rotY.SetRotation(ax2, ry * M_PI / 180.0);
        trsf.Multiply(rotY);
    }

    if (rz != 0.0) {
        gp_Trsf rotZ;
        gp_Ax1 ax3(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
        rotZ.SetRotation(ax3, rz * M_PI / 180.0);
        trsf.Multiply(rotZ);
    }

    if (dx != 0.0 || dy != 0.0 || dz != 0.0) {
        gp_Trsf trans;
        trans.SetTranslation(gp_Vec(dx, dy, dz));
        trsf.Multiply(trans);
    }

    TopoDS_Shape transformedShape = BRepBuilderAPI_Transform(topoShape, trsf, Standard_True).Shape();

    shape->Set(transformedShape);
    myContext->Remove(shape, Standard_False);
    myContext->Display(shape, Standard_True);
    myView->Redraw();
}

// Sketch mode control methods
void OccView::startSketchPointMode()
{
    resetSketchState();
    myCurrentMode = CurAction3d_Sketch_DrawPoint;
    setCursor(Qt::CrossCursor);
}

void OccView::startSketchLineMode()
{
    resetSketchState();
    myCurrentMode = CurAction3d_Sketch_DrawLine;
    setCursor(Qt::CrossCursor);
}

void OccView::startSketchCircleMode()
{
    resetSketchState();
    myCurrentMode = CurAction3d_Sketch_DrawCircle;
    setCursor(Qt::CrossCursor);
}

void OccView::startSketchArcMode()
{
    resetSketchState();
    myCurrentMode = CurAction3d_Sketch_DrawArc;
    setCursor(Qt::CrossCursor);
}

void OccView::stopSketchMode()
{
    resetSketchState();
    myCurrentMode = CurAction3d_Nothing;
    setCursor(Qt::ArrowCursor);
}

// Sketch helper methods
gp_Pnt OccView::convertScreenToWorld(const QPoint& screenPoint)
{
    Standard_Real X, Y, Z;
    myView->Convert(screenPoint.x(), screenPoint.y(), X, Y, Z);

    // Get camera direction for ray casting
    const Handle(Graphic3d_Camera)& c = myView->Camera();
    const gp_Dir& v = c->Direction();

    // Create line from eye through point
    Handle(Geom_Curve) aLine = new Geom_Line(gp_Pnt(X, Y, Z), v);

    // Intersect with XY plane (Z=0)
    gp_Pln prjPln(gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)));
    Handle(Geom_Surface) aPlane = new Geom_Plane(prjPln);
    GeomAPI_IntCS aIntCS(aLine, aPlane);

    if (aIntCS.IsDone() && aIntCS.NbPoints() > 0) {
        return aIntCS.Point(1);
    }

    return gp_Pnt(X, Y, 0.0); // Fallback to Z=0 plane
}

void OccView::updateSketchPreview()
{
    // Remove old preview
    if (!m_previewShape.IsNull()) {
        myContext->Remove(m_previewShape, Standard_True);
        m_previewShape.Nullify();
    }

    if (m_sketchClickCount == 0) return;

    TopoDS_Shape previewShape;
    bool hasPreview = false;

    switch (myCurrentMode) {
    case CurAction3d_Sketch_DrawLine:
        if (m_sketchClickCount >= 1) {
            TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(m_sketchPoint1, m_sketchPoint2);
            previewShape = edge;
            hasPreview = true;
        }
        break;

    case CurAction3d_Sketch_DrawCircle:
        if (m_sketchClickCount >= 1) {
            double radius = m_sketchPoint1.Distance(m_sketchPoint2);
            gp_Circ circle(gp_Ax2(m_sketchPoint1, gp_Dir(0, 0, 1)), radius);
            TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(circle);
            previewShape = edge;
            hasPreview = true;
        }
        break;

    case CurAction3d_Sketch_DrawArc:
        if (m_sketchClickCount >= 2) {
            Handle(Geom_Circle) circle = new Geom_Circle(
                gp_Ax2(m_sketchPoint1, gp_Dir(0, 0, 1)),
                m_sketchPoint1.Distance(m_sketchPoint2));

            double param1 = ElCLib::Parameter(circle->Circ(), m_sketchPoint2);
            double param2 = ElCLib::Parameter(circle->Circ(), m_sketchPoint3);

            if (m_sketchReverseArc) {
                // Reverse direction for major arc
                if (param2 > param1) {
                    param2 -= 2 * M_PI;
                }
            } else {
                // Ensure positive direction for minor arc
                if (param2 < param1) {
                    param2 += 2 * M_PI;
                }
            }

            Handle(Geom_TrimmedCurve) curve = new Geom_TrimmedCurve(
                circle, param1, param2, m_sketchReverseArc);
            TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(curve);
            previewShape = edge;
            hasPreview = true;
        }
        break;

    default:
        break;
    }

    if (hasPreview) {
        m_previewShape = new AIS_Shape(previewShape);
        m_previewShape->SetColor(Quantity_NOC_YELLOW);
        m_previewShape->SetDisplayMode(AIS_WireFrame);
        myContext->Display(m_previewShape, Standard_True);
        myContext->UpdateCurrentViewer();
        myView->Redraw();
    }
}

void OccView::finishSketchShape()
{
    TopoDS_Shape finalShape;
    bool hasShape = false;
    ShapeType shapeType = ShapeType::Unknown;

    switch (myCurrentMode) {
    case CurAction3d_Sketch_DrawPoint:
        if (m_sketchClickCount >= 1) {
            finalShape = BRepBuilderAPI_MakeVertex(m_sketchPoint1);
            hasShape = true;
            shapeType = ShapeType::Point;
        }
        break;

    case CurAction3d_Sketch_DrawLine:
        if (m_sketchClickCount >= 2) {
            finalShape = BRepBuilderAPI_MakeEdge(m_sketchPoint1, m_sketchPoint2);
            hasShape = true;
            shapeType = ShapeType::Line;
        }
        break;

    case CurAction3d_Sketch_DrawCircle:
        if (m_sketchClickCount >= 2) {
            double radius = m_sketchPoint1.Distance(m_sketchPoint2);
            gp_Circ circle(gp_Ax2(m_sketchPoint1, gp_Dir(0, 0, 1)), radius);
            finalShape = BRepBuilderAPI_MakeEdge(circle);
            hasShape = true;
            shapeType = ShapeType::Circle;
        }
        break;

    case CurAction3d_Sketch_DrawArc:
        if (m_sketchClickCount >= 3) {
            Handle(Geom_Circle) circle = new Geom_Circle(
                gp_Ax2(m_sketchPoint1, gp_Dir(0, 0, 1)),
                m_sketchPoint1.Distance(m_sketchPoint2));

            double param1 = ElCLib::Parameter(circle->Circ(), m_sketchPoint2);
            double param2 = ElCLib::Parameter(circle->Circ(), m_sketchPoint3);

            if (m_sketchReverseArc) {
                if (param2 > param1) {
                    param2 -= 2 * M_PI;
                }
            } else {
                if (param2 < param1) {
                    param2 += 2 * M_PI;
                }
            }

            Handle(Geom_TrimmedCurve) curve = new Geom_TrimmedCurve(
                circle, param1, param2, m_sketchReverseArc);
            finalShape = BRepBuilderAPI_MakeEdge(curve);
            hasShape = true;
            shapeType = ShapeType::Arc;
        }
        break;

    default:
        break;
    }

    if (hasShape) {
        // Remove preview shape
        if (!m_previewShape.IsNull()) {
            myContext->Remove(m_previewShape, Standard_True);
            m_previewShape.Nullify();
        }
        
        // Remove arc center point (temporary helper)
        if (!m_arcCenterPoint.IsNull()) {
            myContext->Remove(m_arcCenterPoint, Standard_True);
            m_arcCenterPoint.Nullify();
        }

        // Display final shape with proper color
        Handle(AIS_Shape) aisShape = new AIS_Shape(finalShape);
        switch (shapeType) {
            case ShapeType::Point:
                aisShape->SetColor(Quantity_NOC_RED);
                break;
            case ShapeType::Line:
                aisShape->SetColor(Quantity_NOC_BLUE);
                break;
            case ShapeType::Circle:
                aisShape->SetColor(Quantity_NOC_MAGENTA);
                break;
            case ShapeType::Arc:
                aisShape->SetColor(Quantity_NOC_ORANGE);
                break;
            default:
                aisShape->SetColor(Quantity_NOC_CYAN1);
        }
        aisShape->SetDisplayMode(AIS_Shaded);
        myContext->Display(aisShape, Standard_True);

        // Push undo command
        if (m_commandManager) {
            m_commandManager->executeCommand(new DisplayShapeCommand(myContext, aisShape, "Sketch Shape"));
        }
        // Emit signal
        emit shapeCreated(aisShape, shapeType);

        myContext->ClearSelected(Standard_False);
        myContext->UpdateCurrentViewer();
        myView->Redraw();

        // Reset after completing
        resetSketchState();
    }
}

void OccView::resetSketchState()
{
    // Remove preview shape
    if (!m_previewShape.IsNull()) {
        myContext->Remove(m_previewShape, Standard_True);
        m_previewShape.Nullify();
    }
    
    // Remove arc center point
    if (!m_arcCenterPoint.IsNull()) {
        myContext->Remove(m_arcCenterPoint, Standard_True);
        m_arcCenterPoint.Nullify();
    }

    m_sketchClickCount = 0;
    m_sketchPoint1 = gp_Pnt(0, 0, 0);
    m_sketchPoint2 = gp_Pnt(0, 0, 0);
    m_sketchPoint3 = gp_Pnt(0, 0, 0);
    m_sketchReverseArc = false;
}

QPaintEngine* OccView::paintEngine() const
{
    return nullptr;
}

void OccView::paintEvent(QPaintEvent* /*theEvent*/)
{
    myView->Redraw();
}

void OccView::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    
    if (!m_isInitialized) {
        init();
        m_isInitialized = true;
    }
    
    if (!myView.IsNull()) {
        myView->MustBeResized();
        myView->Redraw();
    }
}

void OccView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (!myView.IsNull()) {
        myView->MustBeResized();
    }
}

void OccView::keyPressEvent(QKeyEvent* theEvent)
{
    if (theEvent->key() == Qt::Key_Escape) {
        if (myCurrentMode >= CurAction3d_Sketch_DrawPoint &&
            myCurrentMode <= CurAction3d_Sketch_DrawArc) {
            myCurrentMode = CurAction3d_DynamicRotation;
            m_sketchClickCount = 0;
            setCursor(Qt::ArrowCursor);
        } else if (myCurrentMode >= CurAction3d_Primitive_CreateBox &&
                   myCurrentMode <= CurAction3d_Primitive_CreateCone) {
            stopPrimitiveMode();
        } else if (myCurrentMode == CurAction3d_Feature_Extrude) {
            stopExtrudeMode();
        } else if (myCurrentMode == CurAction3d_Feature_Revolve) {
            stopRevolveMode();
        }
    }
    QWidget::keyPressEvent(theEvent);
}

void OccView::mousePressEvent(QMouseEvent* theEvent)
{
    if (myView.IsNull()) return;

    if (theEvent->button() == Qt::MidButton) {
        onMButtonDown((theEvent->buttons() | theEvent->modifiers()), theEvent->pos());
        return;
    }

    if (myCurrentMode >= CurAction3d_Sketch_DrawPoint &&
        myCurrentMode <= CurAction3d_Sketch_DrawArc) {

        gp_Pnt clickedPoint = convertScreenToWorld(theEvent->pos());
        bool isLeftButton = (theEvent->button() == Qt::LeftButton);
        bool isRightButton = (theEvent->button() == Qt::RightButton);

        switch (myCurrentMode) {
        case CurAction3d_Sketch_DrawPoint:
            if (isLeftButton) {
                m_sketchPoint1 = clickedPoint;
                m_sketchClickCount = 1;
                finishSketchShape();
            }
            break;

        case CurAction3d_Sketch_DrawLine:
            if (isLeftButton) {
                if (m_sketchClickCount == 0) {
                    m_sketchPoint1 = clickedPoint;
                    m_sketchClickCount = 1;
                } else if (m_sketchClickCount == 1) {
                    m_sketchPoint2 = clickedPoint;
                    m_sketchClickCount = 2;
                    finishSketchShape();
                }
            }
            break;

        case CurAction3d_Sketch_DrawCircle:
            if (isLeftButton) {
                if (m_sketchClickCount == 0) {
                    m_sketchPoint1 = clickedPoint;
                    m_sketchClickCount = 1;
                } else if (m_sketchClickCount == 1) {
                    m_sketchPoint2 = clickedPoint;
                    m_sketchClickCount = 2;
                    finishSketchShape();
                }
            }
            break;

        case CurAction3d_Sketch_DrawArc:
            if (isLeftButton) {
                if (m_sketchClickCount == 0) {
                    m_sketchPoint1 = clickedPoint;
                    m_sketchClickCount = 1;
                    
                    m_arcCenterPoint = new AIS_Shape(BRepBuilderAPI_MakeVertex(m_sketchPoint1).Shape());
                    m_arcCenterPoint->SetColor(Quantity_NOC_RED);
                    myContext->Display(m_arcCenterPoint, Standard_True);
                    myContext->UpdateCurrentViewer();
                    myView->Redraw();
                } else if (m_sketchClickCount == 1) {
                    m_sketchPoint2 = clickedPoint;
                    m_sketchClickCount = 2;
                } else if (m_sketchClickCount == 2) {
                    m_sketchPoint3 = clickedPoint;
                    m_sketchClickCount = 3;
                    finishSketchShape();
                }
            } else if (isRightButton && m_sketchClickCount == 2) {
                m_sketchPoint3 = clickedPoint;
                m_sketchReverseArc = true;
                m_sketchClickCount = 3;
                finishSketchShape();
            }
            break;

        default:
            break;
        }

        return;
    }

    if (myCurrentMode >= CurAction3d_Primitive_CreateBox &&
        myCurrentMode <= CurAction3d_Primitive_CreateCone) {
        handlePrimitiveCreation(theEvent);
        return;
    }

    if (myCurrentMode == CurAction3d_Feature_Extrude) {
        if (theEvent->button() == Qt::LeftButton) {
            myContext->MoveTo(theEvent->pos().x(), theEvent->pos().y(), myView, true);
            
            if (myContext->HasDetected()) {
                Handle(AIS_InteractiveObject) detected = myContext->DetectedInteractive();
                Handle(AIS_Shape) shape = Handle(AIS_Shape)::DownCast(detected);
                
                if (!shape.IsNull()) {
                    performExtrude(shape);
                    stopExtrudeMode();
                }
            }
        } else if (theEvent->button() == Qt::RightButton) {
            stopExtrudeMode();
        }
        return;
    }

    if (myCurrentMode == CurAction3d_Feature_Revolve) {
        if (theEvent->button() == Qt::LeftButton) {
            myContext->MoveTo(theEvent->pos().x(), theEvent->pos().y(), myView, true);
            
            if (myContext->HasDetected()) {
                Handle(AIS_InteractiveObject) detected = myContext->DetectedInteractive();
                Handle(AIS_Shape) shape = Handle(AIS_Shape)::DownCast(detected);
                
                if (!shape.IsNull()) {
                    performRevolve(shape);
                    stopRevolveMode();
                }
            }
        } else if (theEvent->button() == Qt::RightButton) {
            stopRevolveMode();
        }
        return;
    }

    if (myCurrentMode == CurAction3d_Feature_Sweep) {
        if (theEvent->button() == Qt::LeftButton) {
            myContext->MoveTo(theEvent->pos().x(), theEvent->pos().y(), myView, true);
            
            if (myContext->HasDetected()) {
                Handle(AIS_InteractiveObject) detected = myContext->DetectedInteractive();
                Handle(AIS_Shape) shape = Handle(AIS_Shape)::DownCast(detected);
                
                if (!shape.IsNull()) {
                    if (m_sweepProfile.IsNull()) {
                        m_sweepProfile = shape;
                        qDebug() << "Sweep mode: Profile selected, now select path";
                    } else {
                        performSweep(m_sweepProfile, shape);
                        stopSweepMode();
                    }
                }
            }
        } else if (theEvent->button() == Qt::RightButton) {
            stopSweepMode();
        }
        return;
    }

    if (theEvent->button() == Qt::LeftButton) {
        onLButtonDown((theEvent->buttons() | theEvent->modifiers()), theEvent->pos());

        myContext->MoveTo(theEvent->pos().x(), theEvent->pos().y(), myView, true);

        if (myContext->HasDetected()) {
            myContext->SelectDetected();
        }
    }
    else if (theEvent->button() == Qt::RightButton) {
        onRButtonDown((theEvent->buttons() | theEvent->modifiers()), theEvent->pos());
    }
}

void OccView::mouseReleaseEvent(QMouseEvent* theEvent)
{
    if (myView.IsNull()) return;

    if (myCurrentMode >= CurAction3d_Sketch_DrawPoint &&
        myCurrentMode <= CurAction3d_Sketch_DrawArc) {
        return;
    }

    if (theEvent->button() == Qt::LeftButton) {
        onLButtonUp(theEvent->buttons() | theEvent->modifiers(), theEvent->pos());
    }
    else if (theEvent->button() == Qt::MidButton) {
        onMButtonUp(theEvent->buttons() | theEvent->modifiers(), theEvent->pos());
    }
    else if (theEvent->button() == Qt::RightButton) {
        onRButtonUp(theEvent->buttons() | theEvent->modifiers(), theEvent->pos());
    }
}

void OccView::mouseMoveEvent(QMouseEvent* theEvent)
{
    if (myView.IsNull()) return;

    emit mouseMoved(theEvent->pos());

    if (theEvent->buttons() & Qt::MidButton) {
        switch (myCurrentMode) {
        case CurAction3d_DynamicRotation:
        case CurAction3d_Sketch_DrawPoint:
        case CurAction3d_Sketch_DrawLine:
        case CurAction3d_Sketch_DrawCircle:
        case CurAction3d_Sketch_DrawArc:
            myView->Rotation(theEvent->pos().x(), theEvent->pos().y());
            break;

        case CurAction3d_DynamicPanning:
            myView->Pan(theEvent->pos().x() - myXmax, myYmax - theEvent->pos().y());
            myXmax = theEvent->pos().x();
            myYmax = theEvent->pos().y();
            break;

        case CurAction3d_DynamicZooming:
        case CurAction3d_WindowZooming:
            myView->Zoom(myXmin, myYmin, theEvent->pos().x(), theEvent->pos().y());
            break;

        default:
            break;
        }
        return;
    }

    if (myCurrentMode >= CurAction3d_Sketch_DrawPoint &&
        myCurrentMode <= CurAction3d_Sketch_DrawArc) {

        gp_Pnt mousePoint = convertScreenToWorld(theEvent->pos());

        switch (myCurrentMode) {
        case CurAction3d_Sketch_DrawLine:
            if (m_sketchClickCount == 1) {
                m_sketchPoint2 = mousePoint;
                updateSketchPreview();
            }
            break;

        case CurAction3d_Sketch_DrawCircle:
            if (m_sketchClickCount == 1) {
                m_sketchPoint2 = mousePoint;
                updateSketchPreview();
            }
            break;

        case CurAction3d_Sketch_DrawArc:
            if (m_sketchClickCount == 2) {
                m_sketchPoint3 = mousePoint;
                updateSketchPreview();
            }
            break;

        default:
            break;
        }

        return;
    }

    if (myCurrentMode >= CurAction3d_Primitive_CreateBox &&
        myCurrentMode <= CurAction3d_Primitive_CreateCone) {
        handlePrimitiveMouseMove(theEvent);
        return;
    }

    onMouseMove(theEvent->buttons(), theEvent->pos());
}

void OccView::wheelEvent(QWheelEvent* theEvent)
{
    if (myView.IsNull()) return;

    onMouseWheel(theEvent->buttons(), theEvent->angleDelta().y(), theEvent->pos());
}

void OccView::onLButtonDown(const int /*theFlags*/, const QPoint thePoint)
{
    myXmin = thePoint.x();
    myYmin = thePoint.y();
    myXmax = thePoint.x();
    myYmax = thePoint.y();
}

void OccView::onMButtonDown(const int /*theFlags*/, const QPoint thePoint)
{
    myXmin = thePoint.x();
    myYmin = thePoint.y();
    myXmax = thePoint.x();
    myYmax = thePoint.y();

    if (myCurrentMode == CurAction3d_DynamicRotation) {
        myView->StartRotation(thePoint.x(), thePoint.y());
    }
}

void OccView::onRButtonDown(const int /*theFlags*/, const QPoint /*thePoint*/)
{
}

void OccView::onLButtonUp(const int theFlags, const QPoint thePoint)
{
    Q_UNUSED(theFlags);
    if (myView.IsNull()) return;

    if (myRectBand) {
        myRectBand->hide();
    }

    if (myXmin == myXmax && myYmin == myYmax) {
        inputEvent(thePoint.x(), thePoint.y());
    }
}

void OccView::onMButtonUp(const int /*theFlags*/, const QPoint thePoint)
{
    if (thePoint.x() == myXmin && thePoint.y() == myYmin) {
        panByMiddleButton(thePoint);
    }
}

void OccView::onRButtonUp(const int /*theFlags*/, const QPoint thePoint)
{
    popup(thePoint.x(), thePoint.y());
}

void OccView::popup(const int /*x*/, const int /*y*/)
{
}

void OccView::panByMiddleButton(const QPoint& thePoint)
{
    Standard_Integer aCenterX = 0;
    Standard_Integer aCenterY = 0;

    QSize aSize = size();

    aCenterX = aSize.width() / 2;
    aCenterY = aSize.height() / 2;

    myView->Pan(aCenterX - thePoint.x(), thePoint.y() - aCenterY);
}

void OccView::drawRubberBand(const int minX, const int minY, const int maxX, const int maxY)
{
    QRect aRect;

    (minX < maxX) ? (aRect.setX(minX)) : (aRect.setX(maxX));
    (minY < maxY) ? (aRect.setY(minY)) : (aRect.setY(maxY));

    aRect.setWidth(abs(maxX - minX));
    aRect.setHeight(abs(maxY - minY));

    if (!myRectBand)
    {
        myRectBand = new QRubberBand(QRubberBand::Rectangle, this);
        myRectBand->setStyle(QStyleFactory::create("windows"));
    }

    myRectBand->setGeometry(aRect);
    myRectBand->show();
}

void OccView::dragEvent(const int x, const int y)
{
    myContext->Select(myXmin, myYmin, x, y, myView, Standard_True);
    emit selectionChanged();
}

void OccView::inputEvent(const int x, const int y)
{
    Q_UNUSED(x);
    Q_UNUSED(y);

    myContext->Select(Standard_True);
    emit selectionChanged();
}

void OccView::moveEvent(const int x, const int y)
{
    myContext->MoveTo(x, y, myView, Standard_True);
}

void OccView::multiMoveEvent(const int x, const int y)
{
    myContext->MoveTo(x, y, myView, Standard_True);
}

void OccView::onMouseMove(const int theFlags, const QPoint thePoint)
{
    if (myView.IsNull()) return;

    if (theFlags & Qt::LeftButton) {
        drawRubberBand(myXmin, myYmin, thePoint.x(), thePoint.y());
        dragEvent(thePoint.x(), thePoint.y());
    }

    if (theFlags & Qt::ControlModifier) {
        multiMoveEvent(thePoint.x(), thePoint.y());
    }
    else {
        moveEvent(thePoint.x(), thePoint.y());
    }

    if (theFlags & Qt::MidButton) {
        switch (myCurrentMode) {
        case CurAction3d_DynamicRotation:
            myView->Rotation(thePoint.x(), thePoint.y());
            break;

        case CurAction3d_DynamicZooming:
        case CurAction3d_WindowZooming:
            myView->Zoom(myXmin, myYmin, thePoint.x(), thePoint.y());
            break;

        case CurAction3d_DynamicPanning:
            myView->Pan(thePoint.x() - myXmax, myYmax - thePoint.y());
            myXmax = thePoint.x();
            myYmax = thePoint.y();
            break;

        default:
            break;
        }
    }
}

void OccView::onMouseWheel(const int theFlags, const int theDelta, const QPoint thePoint)
{
    Q_UNUSED(theFlags);
    if (myView.IsNull()) return;

    Standard_Integer aFactor = 16;

    Standard_Integer aX = thePoint.x();
    Standard_Integer aY = thePoint.y();

    if (theDelta > 0)
    {
        aX += aFactor;
        aY += aFactor;
    }
    else
    {
        aX -= aFactor;
        aY -= aFactor;
    }

    myView->Zoom(thePoint.x(), thePoint.y(), aX, aY);
}

void OccView::pan(void)
{
    myCurrentMode = CurAction3d_DynamicPanning;
}

void OccView::fitAll(void)
{
    if (!myView.IsNull()) {
        myView->FitAll();
        myView->Redraw();
    }
}

void OccView::reset(void)
{
    if (!myView.IsNull()) {
        myView->Reset();
        myView->Redraw();
    }
}

void OccView::cancelSketch(void)
{
    if (myCurrentMode >= CurAction3d_Sketch_DrawPoint &&
        myCurrentMode <= CurAction3d_Sketch_DrawArc) {
        myCurrentMode = CurAction3d_DynamicRotation;
        m_sketchClickCount = 0;
        setCursor(Qt::ArrowCursor);
    }
}

void OccView::drawPoint(double x, double y)
{
    if (myView.IsNull()) return;
    
    gp_Pnt point(x, y, 0.0);
    TopoDS_Shape shape = BRepBuilderAPI_MakeVertex(point).Shape();
    
    Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
    aisShape->SetColor(Quantity_NOC_RED);
    myContext->Display(aisShape, Standard_True);
    myContext->UpdateCurrentViewer();
    myView->Redraw();
    if (m_commandManager) {
        m_commandManager->executeCommand(new DisplayShapeCommand(myContext, aisShape, "Draw Point"));
    }
    myContext->ClearSelected(Standard_False);
    myContext->UpdateCurrentViewer();
    myView->Redraw();
    emit shapeCreated(aisShape, ShapeType::Point);
}

void OccView::drawLine(double x1, double y1, double x2, double y2)
{
    if (myView.IsNull()) return;
    
    gp_Pnt p1(x1, y1, 0.0);
    gp_Pnt p2(x2, y2, 0.0);
    TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(p1, p2);
    TopoDS_Shape shape = edge;
    
    Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
    aisShape->SetColor(Quantity_NOC_BLUE);
    myContext->Display(aisShape, Standard_True);
    myContext->UpdateCurrentViewer();
    myView->Redraw();
    if (m_commandManager) {
        m_commandManager->executeCommand(new DisplayShapeCommand(myContext, aisShape, "Draw Shape"));
    }
    myContext->ClearSelected(Standard_False);
    myContext->UpdateCurrentViewer();
    myView->Redraw();
    emit shapeCreated(aisShape, ShapeType::Line);
}

void OccView::drawPolyline(const QVector<QPair<double, double>>& points)
{
    if (myView.IsNull() || points.size() < 2) return;
    
    BRepBuilderAPI_MakeWire wireMaker;
    
    for (int i = 0; i < points.size() - 1; ++i) {
        gp_Pnt p1(points[i].first, points[i].second, 0.0);
        gp_Pnt p2(points[i+1].first, points[i+1].second, 0.0);
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(p1, p2);
        wireMaker.Add(edge);
    }
    
    TopoDS_Shape shape = wireMaker.Shape();
    
    Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
    aisShape->SetColor(Quantity_NOC_BLUE);
    myContext->Display(aisShape, Standard_True);
    myContext->UpdateCurrentViewer();
    myView->Redraw();
    if (m_commandManager) {
        m_commandManager->executeCommand(new DisplayShapeCommand(myContext, aisShape, "Draw Shape"));
    }
    myContext->ClearSelected(Standard_False);
    myContext->UpdateCurrentViewer();
    myView->Redraw();
    emit shapeCreated(aisShape, ShapeType::Line);
}

void OccView::drawPolygon(const QVector<QPair<double, double>>& points)
{
    if (myView.IsNull() || points.size() < 3) return;
    
    BRepBuilderAPI_MakeWire wireMaker;
    
    for (int i = 0; i < points.size(); ++i) {
        gp_Pnt p1(points[i].first, points[i].second, 0.0);
        int nextIdx = (i + 1) % points.size();
        gp_Pnt p2(points[nextIdx].first, points[nextIdx].second, 0.0);
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(p1, p2);
        wireMaker.Add(edge);
    }
    
    TopoDS_Shape shape = wireMaker.Shape();
    
    Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
    aisShape->SetColor(Quantity_NOC_BLUE);
    myContext->Display(aisShape, Standard_True);
    myContext->UpdateCurrentViewer();
    myView->Redraw();
    if (m_commandManager) {
        m_commandManager->executeCommand(new DisplayShapeCommand(myContext, aisShape, "Draw Shape"));
    }
    myContext->ClearSelected(Standard_False);
    myContext->UpdateCurrentViewer();
    myView->Redraw();
    emit shapeCreated(aisShape, ShapeType::Line);
}

void OccView::drawCircle(double cx, double cy, double radius)
{
    if (myView.IsNull()) return;
    
    gp_Pnt center(cx, cy, 0.0);
    gp_Circ circle(gp_Ax2(center, gp_Dir(0, 0, 1)), radius);
    TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(circle);
    TopoDS_Shape shape = edge;
    
    Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
    aisShape->SetColor(Quantity_NOC_MAGENTA);
    myContext->Display(aisShape, Standard_True);
    myContext->UpdateCurrentViewer();
    myView->Redraw();
    if (m_commandManager) {
        m_commandManager->executeCommand(new DisplayShapeCommand(myContext, aisShape, "Draw Shape"));
    }
    myContext->ClearSelected(Standard_False);
    myContext->UpdateCurrentViewer();
    myView->Redraw();
    emit shapeCreated(aisShape, ShapeType::Circle);
}

void OccView::drawArc(double cx, double cy, double radius, double startAngle, double sweepAngle)
{
    if (myView.IsNull()) return;
    
    gp_Pnt center(cx, cy, 0.0);
    
    double startRad = startAngle * M_PI / 180.0;
    double sweepRad = sweepAngle * M_PI / 180.0;
    double midRad = startRad + sweepRad / 2.0;
    double endRad = startRad + sweepRad;
    
    gp_Pnt startPoint(cx + radius * cos(startRad), cy + radius * sin(startRad), 0.0);
    gp_Pnt midPoint(cx + radius * cos(midRad), cy + radius * sin(midRad), 0.0);
    gp_Pnt endPoint(cx + radius * cos(endRad), cy + radius * sin(endRad), 0.0);
    
    GC_MakeArcOfCircle arcMaker(startPoint, midPoint, endPoint);
    if (!arcMaker.IsDone()) {
        return;
    }
    
    TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(arcMaker.Value());
    TopoDS_Shape shape = edge;
    
    Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
    aisShape->SetColor(Quantity_NOC_ORANGE);
    myContext->Display(aisShape, Standard_True);
    myContext->UpdateCurrentViewer();
    myView->Redraw();
    if (m_commandManager) {
        m_commandManager->executeCommand(new DisplayShapeCommand(myContext, aisShape, "Draw Shape"));
    }
    myContext->ClearSelected(Standard_False);
    myContext->UpdateCurrentViewer();
    myView->Redraw();
    emit shapeCreated(aisShape, ShapeType::Arc);
}

void OccView::eraseSelected(void)
{
    if (myContext.IsNull()) return;
    
    QList<Handle(AIS_InteractiveObject)> selectedObjects;
    for (myContext->InitSelected(); myContext->MoreSelected(); myContext->NextSelected())
    {
        Handle(AIS_InteractiveObject) selectedObject = myContext->SelectedInteractive();
        if (myContext->IsDisplayed(selectedObject))
        {
            selectedObjects.append(selectedObject);
        }
    }
    
    for (const Handle(AIS_InteractiveObject)& obj : selectedObjects)
    {
        myContext->Erase(obj, false);
    }
    myContext->UpdateCurrentViewer();
}

QList<Handle(AIS_InteractiveObject)> OccView::getSelectedObjects(void)
{
    QList<Handle(AIS_InteractiveObject)> selectedObjects;
    if (myContext.IsNull()) return selectedObjects;
    
    for (myContext->InitSelected(); myContext->MoreSelected(); myContext->NextSelected())
    {
        Handle(AIS_InteractiveObject) selectedObject = myContext->SelectedInteractive();
        if (myContext->IsDisplayed(selectedObject))
        {
            selectedObjects.append(selectedObject);
        }
    }
    return selectedObjects;
}

void OccView::selectShape(const Handle(AIS_Shape)& shape)
{
    if (myContext.IsNull() || shape.IsNull()) return;
    
    for (myContext->InitSelected(); myContext->MoreSelected(); myContext->NextSelected())
    {
        Handle(AIS_InteractiveObject) obj = myContext->SelectedInteractive();
        myContext->Unhilight(obj, Standard_False);
    }
    
    myContext->ClearSelected(Standard_False);
    myContext->SetSelected(shape, Standard_True);
    
    Handle(Prs3d_Drawer) highlightStyle = new Prs3d_Drawer();
    highlightStyle->SetColor(Quantity_NOC_GREEN);
    myContext->HilightWithColor(shape, highlightStyle, Standard_True);
}

void OccView::zoom(void)
{
    myCurrentMode = CurAction3d_WindowZooming;
}

void OccView::rotate(void)
{
    myCurrentMode = CurAction3d_DynamicRotation;
}

void OccView::top(void)
{
    if (!myView.IsNull()) {
        myView->SetProj(V3d_TypeOfOrientation_Zup_Top);
        myView->Redraw();
    }
}

void OccView::bottom(void)
{
    if (!myView.IsNull()) {
        myView->SetProj(V3d_TypeOfOrientation_Zup_Bottom);
        myView->Redraw();
    }
}

void OccView::left(void)
{
    if (!myView.IsNull()) {
        myView->SetProj(V3d_TypeOfOrientation_Zup_Left);
        myView->Redraw();
    }
}

void OccView::right(void)
{
    if (!myView.IsNull()) {
        myView->SetProj(V3d_TypeOfOrientation_Zup_Right);
        myView->Redraw();
    }
}

void OccView::front(void)
{
    if (!myView.IsNull()) {
        myView->SetProj(V3d_TypeOfOrientation_Zup_Front);
        myView->Redraw();
    }
}

void OccView::back(void)
{
    if (!myView.IsNull()) {
        myView->SetProj(V3d_TypeOfOrientation_Zup_Back);
        myView->Redraw();
    }
}

void OccView::shaded(void)
{
    if (!myContext.IsNull()) {
        myContext->SetDisplayMode(AIS_Shaded, Standard_True);
        myContext->UpdateCurrentViewer();
        myView->Redraw();
    }
}

void OccView::wireframe(void)
{
    if (!myContext.IsNull()) {
        myContext->SetDisplayMode(AIS_WireFrame, Standard_True);
        myContext->UpdateCurrentViewer();
        myView->Redraw();
    }
}

void OccView::onViewModelShadedChanged(bool isShaded)
{
    if (isShaded) {
        shaded();
    } else {
        wireframe();
    }
}

void OccView::init(void)
{
    Handle(Aspect_DisplayConnection) aDisplayConnection = new Aspect_DisplayConnection();

    if (GetGraphicDriver().IsNull()) {
        GetGraphicDriver() = new OpenGl_GraphicDriver(aDisplayConnection);
    }

    WId window_handle = (WId)winId();

#if defined(_WIN32) || defined(_WIN64) || defined(WNT)
    Handle(WNT_Window) wind = new WNT_Window((Aspect_Handle)window_handle);
#elif defined(__APPLE__) && !defined(MACOSX_USE_GLX)
    Handle(Cocoa_Window) wind = new Cocoa_Window((NSView*)window_handle);
#else
    Handle(Xw_Window) wind = new Xw_Window(aDisplayConnection, (Window)window_handle);
#endif

    myViewer = new V3d_Viewer(GetGraphicDriver());
    myView = myViewer->CreateView();
    myView->SetWindow(wind);

    if (!wind->IsMapped()) {
        wind->Map();
    }

    myContext = new AIS_InteractiveContext(myViewer);

    myViewer->SetDefaultLights();
    myViewer->SetLightOn();
    myViewer->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);

    myView->SetBgGradientColors(Quantity_NOC_SLATEGRAY3, Quantity_NOC_SLATEGRAY4, Aspect_GradientFillMethod_Horizontal, false);

    myView->MustBeResized();
    myView->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_GOLD, 0.08, V3d_ZBUFFER);
    myView->SetLightOn(new V3d_DirectionalLight(V3d_Zneg, Quantity_NOC_WHITE, Standard_True));
    myContext->SetDisplayMode(AIS_Shaded, Standard_True);
    myView->SetLightOn(new V3d_AmbientLight(Quantity_NOC_WHITE));

    myView->SetLightOn(new V3d_DirectionalLight(V3d_Xneg, Quantity_NOC_WHITE, Standard_True));
    myView->SetLightOn(new V3d_DirectionalLight(V3d_Yneg, Quantity_NOC_WHITE, Standard_True));

    myView->SetShadingModel(Graphic3d_TOSM_FRAGMENT);
    myViewer->SetDefaultShadingModel(Graphic3d_TOSM_FRAGMENT);

    Handle(AIS_ViewCube) aViewCube = new AIS_ViewCube();
    aViewCube->SetTransformPersistence(
        new Graphic3d_TransformPers(
            Graphic3d_TMF_TriedronPers,
            Aspect_TOTP_RIGHT_UPPER,
            Graphic3d_Vec2i(100, 100)));
    myContext->Display(aViewCube, Standard_True);
    myContext->SetDisplayMode(aViewCube, AIS_WireFrame, Standard_True);
    
    Handle(Prs3d_Drawer) highlightStyle = myContext->HighlightStyle(Prs3d_TypeOfHighlight_Selected);
    highlightStyle->SetColor(Quantity_NOC_GREEN);
}

// ========================================
// Primitive creation methods
// ========================================

void OccView::startPrimitiveBoxMode()
{
    myCurrentMode = CurAction3d_Primitive_CreateBox;
    m_primitivePhase = PrimitivePhase_Start;
    setCursor(Qt::CrossCursor);
}

void OccView::startPrimitiveSphereMode()
{
    myCurrentMode = CurAction3d_Primitive_CreateSphere;
    m_primitivePhase = PrimitivePhase_Start;
    setCursor(Qt::CrossCursor);
}

void OccView::startPrimitiveCylinderMode()
{
    myCurrentMode = CurAction3d_Primitive_CreateCylinder;
    m_primitivePhase = PrimitivePhase_Start;
    setCursor(Qt::CrossCursor);
}

void OccView::startPrimitiveConeMode()
{
    myCurrentMode = CurAction3d_Primitive_CreateCone;
    m_primitivePhase = PrimitivePhase_Start;
    setCursor(Qt::CrossCursor);
}

void OccView::stopPrimitiveMode()
{
    if (myCurrentMode >= CurAction3d_Primitive_CreateBox &&
        myCurrentMode <= CurAction3d_Primitive_CreateCone) {
        myCurrentMode = CurAction3d_DynamicRotation;
        setCursor(Qt::ArrowCursor);
        
        if (!m_previewShape.IsNull()) {
            myContext->Remove(m_previewShape, Standard_True);
            m_previewShape.Nullify();
        }
    }
}

void OccView::startExtrudeMode()
{
    myCurrentMode = CurAction3d_Feature_Extrude;
    setCursor(Qt::CrossCursor);
    qDebug() << "Extrude mode: Select a sketch to extrude";
}

void OccView::stopExtrudeMode()
{
    if (myCurrentMode == CurAction3d_Feature_Extrude) {
        myCurrentMode = CurAction3d_DynamicRotation;
        setCursor(Qt::ArrowCursor);
    }
}

void OccView::startRevolveMode()
{
    myCurrentMode = CurAction3d_Feature_Revolve;
    setCursor(Qt::CrossCursor);
    qDebug() << "Revolve mode: Select a sketch to revolve";
}

void OccView::stopRevolveMode()
{
    if (myCurrentMode == CurAction3d_Feature_Revolve) {
        myCurrentMode = CurAction3d_DynamicRotation;
        setCursor(Qt::ArrowCursor);
    }
}

void OccView::startSweepMode()
{
    myCurrentMode = CurAction3d_Feature_Sweep;
    m_sweepProfile.Nullify();
    setCursor(Qt::CrossCursor);
    qDebug() << "Sweep mode: First select profile, then select path";
}

void OccView::stopSweepMode()
{
    if (myCurrentMode == CurAction3d_Feature_Sweep) {
        myCurrentMode = CurAction3d_DynamicRotation;
        m_sweepProfile.Nullify();
        setCursor(Qt::ArrowCursor);
    }
}

void OccView::performSweep(const Handle(AIS_Shape)& profileShape, const Handle(AIS_Shape)& pathShape)
{
    TopoDS_Shape profile = profileShape->Shape();
    TopoDS_Shape path = pathShape->Shape();

    TopoDS_Wire profileWire;
    if (profile.ShapeType() == TopAbs_WIRE) {
        profileWire = TopoDS::Wire(profile);
    } else if (profile.ShapeType() == TopAbs_EDGE) {
        TopoDS_Edge edge = TopoDS::Edge(profile);
        BRepBuilderAPI_MakeWire wireMaker(edge);
        profileWire = TopoDS::Wire(wireMaker.Shape());
    } else {
        QMessageBox::warning(this, "Sweep Error", "Profile must be a wire or edge.", QMessageBox::Ok);
        return;
    }

    TopoDS_Wire pathWire;
    if (path.ShapeType() == TopAbs_WIRE) {
        pathWire = TopoDS::Wire(path);
    } else if (path.ShapeType() == TopAbs_EDGE) {
        TopoDS_Edge edge = TopoDS::Edge(path);
        BRepBuilderAPI_MakeWire wireMaker(edge);
        pathWire = TopoDS::Wire(wireMaker.Shape());
    } else {
        QMessageBox::warning(this, "Sweep Error", "Path must be a wire or edge.", QMessageBox::Ok);
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Sweep Settings");
    dialog.resize(350, 280);

    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    QFont labelFont("Arial", 14);
    QFont radioFont("Arial", 13);

    QLabel* titleLabel = new QLabel("Choose Sweep Mode:", &dialog);
    titleLabel->setFont(labelFont);
    titleLabel->setStyleSheet("font-weight: bold; color: #333;");
    mainLayout->addWidget(titleLabel);

    QRadioButton* faceRadio = new QRadioButton("Sweep Face (Solid)", &dialog);
    faceRadio->setFont(radioFont);
    faceRadio->setChecked(true);
    faceRadio->setStyleSheet("color: #333;");
    mainLayout->addWidget(faceRadio);

    QRadioButton* wireRadio = new QRadioButton("Sweep Wire (Surface)", &dialog);
    wireRadio->setFont(radioFont);
    wireRadio->setStyleSheet("color: #333;");
    mainLayout->addWidget(wireRadio);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    QPushButton* okBtn = buttonBox->button(QDialogButtonBox::Ok);
    QPushButton* cancelBtn = buttonBox->button(QDialogButtonBox::Cancel);
    okBtn->setFont(QFont("Arial", 13));
    cancelBtn->setFont(QFont("Arial", 13));
    okBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 8px 20px; border: none; border-radius: 6px; } QPushButton:hover { background-color: #45a049; }");
    cancelBtn->setStyleSheet("QPushButton { background-color: #f44336; color: white; padding: 8px 20px; border: none; border-radius: 6px; } QPushButton:hover { background-color: #da190b; }");
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        bool sweepFace = faceRadio->isChecked();

        if (sweepFace) {
            BRepBuilderAPI_MakeFace faceMaker(profileWire);
            if (!faceMaker.IsDone()) {
                QMessageBox::warning(this, "Sweep Error", "Cannot sweep face because the profile is not closed.", QMessageBox::Ok);
                return;
            }
            profile = faceMaker.Shape();
        }

        BRepOffsetAPI_MakePipe pipeMaker(pathWire, profile);
        if (!pipeMaker.IsDone()) {
            QMessageBox::warning(this, "Sweep Error", "Failed to create sweep.", QMessageBox::Ok);
            return;
        }

        TopoDS_Shape sweptShape = pipeMaker.Shape();

        if (!sweptShape.IsNull()) {
            Handle(AIS_Shape) aisShape = new AIS_Shape(sweptShape);
            aisShape->SetColor(Quantity_NOC_YELLOW);
            aisShape->SetDisplayMode(AIS_Shaded);
            myContext->Display(aisShape, Standard_True);

            if (m_commandManager) {
                m_commandManager->executeCommand(new DisplayShapeCommand(myContext, aisShape, "Sweep"));
            }

            emit shapeCreated(aisShape, ShapeType::Sweep);
        }
    }
}

void OccView::performExtrude(const Handle(AIS_Shape)& sketchShape)
{
    TopoDS_Shape basisShape = sketchShape->Shape();
    
    QDialog dialog(this);
    dialog.setWindowTitle("Extrude Settings");
    dialog.resize(350, 250);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    QFont labelFont("Arial", 14);
    QFont radioFont("Arial", 13);
    QFont spinFont("Arial", 14);
    
    QLabel* titleLabel = new QLabel("Choose Extrude Mode:", &dialog);
    titleLabel->setFont(labelFont);
    titleLabel->setStyleSheet("font-weight: bold; color: #333;");
    mainLayout->addWidget(titleLabel);
    
    QRadioButton* faceRadio = new QRadioButton("Extrude Face (Solid)", &dialog);
    faceRadio->setFont(radioFont);
    faceRadio->setChecked(true);
    faceRadio->setStyleSheet("color: #333;");
    mainLayout->addWidget(faceRadio);
    
    QRadioButton* wireRadio = new QRadioButton("Extrude Wire (Surface)", &dialog);
    wireRadio->setFont(radioFont);
    wireRadio->setStyleSheet("color: #333;");
    mainLayout->addWidget(wireRadio);
    
    QLabel* heightLabel = new QLabel("Extrude Height:", &dialog);
    heightLabel->setFont(labelFont);
    heightLabel->setStyleSheet("font-weight: bold; color: #333;");
    mainLayout->addWidget(heightLabel);
    
    QDoubleSpinBox* heightSpin = new QDoubleSpinBox(&dialog);
    heightSpin->setFont(spinFont);
    heightSpin->setRange(-1000.0, 1000.0);
    heightSpin->setValue(5.0);
    heightSpin->setDecimals(3);
    heightSpin->setFixedHeight(35);
    heightSpin->setStyleSheet("QDoubleSpinBox { padding: 5px; border: 2px solid #ccc; border-radius: 6px; }");
    mainLayout->addWidget(heightSpin);
    
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    QPushButton* okBtn = buttonBox->button(QDialogButtonBox::Ok);
    QPushButton* cancelBtn = buttonBox->button(QDialogButtonBox::Cancel);
    okBtn->setFont(QFont("Arial", 13));
    cancelBtn->setFont(QFont("Arial", 13));
    okBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 8px 20px; border: none; border-radius: 6px; } QPushButton:hover { background-color: #45a049; }");
    cancelBtn->setStyleSheet("QPushButton { background-color: #f44336; color: white; padding: 8px 20px; border: none; border-radius: 6px; } QPushButton:hover { background-color: #da190b; }");
    mainLayout->addWidget(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        bool extrudeFace = faceRadio->isChecked();
        
        if (extrudeFace) {
            TopoDS_Wire wire;
            if (basisShape.ShapeType() == TopAbs_WIRE) {
                wire = TopoDS::Wire(basisShape);
            } else if (basisShape.ShapeType() == TopAbs_EDGE) {
                TopoDS_Edge edge = TopoDS::Edge(basisShape);
                BRepBuilderAPI_MakeWire wireMaker(edge);
                wire = TopoDS::Wire(wireMaker.Shape());
            } else {
                QMessageBox::warning(this, "Extrude Error", "Cannot create face from this shape type.", QMessageBox::Ok);
                return;
            }
            
            BRepBuilderAPI_MakeFace faceMaker(wire);
            if (!faceMaker.IsDone()) {
                QMessageBox::warning(this, "Extrude Error", "Cannot extrude face because the sketch is not closed.", QMessageBox::Ok);
                return;
            }
            basisShape = faceMaker.Shape();
        }
        
        double height = heightSpin->value();
        double absHeight = fabs(height);
        if (absHeight < 0.001) absHeight = 0.001;
        
        gp_Vec vec(0, 0, height);
        
        TopoDS_Shape extrudedShape = BRepPrimAPI_MakePrism(basisShape, vec).Shape();
        
        if (!extrudedShape.IsNull()) {
            Handle(AIS_Shape) aisShape = new AIS_Shape(extrudedShape);
            aisShape->SetColor(Quantity_NOC_YELLOW);
            aisShape->SetDisplayMode(AIS_Shaded);
            myContext->Display(aisShape, Standard_True);
            
            if (m_commandManager) {
                m_commandManager->executeCommand(new DisplayShapeCommand(myContext, aisShape, "Extrude"));
            }
            
            emit shapeCreated(aisShape, ShapeType::Extrude);
        }
    }
}

void OccView::performRevolve(const Handle(AIS_Shape)& sketchShape)
{
    TopoDS_Shape basisShape = sketchShape->Shape();
    
    QDialog dialog(this);
    dialog.setWindowTitle("Revolve Settings");
    dialog.resize(350, 320);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    QFont labelFont("Arial", 14);
    QFont radioFont("Arial", 13);
    QFont spinFont("Arial", 14);
    
    QLabel* titleLabel = new QLabel("Choose Revolve Mode:", &dialog);
    titleLabel->setFont(labelFont);
    titleLabel->setStyleSheet("font-weight: bold; color: #333;");
    mainLayout->addWidget(titleLabel);
    
    QRadioButton* faceRadio = new QRadioButton("Revolve Face (Solid)", &dialog);
    faceRadio->setFont(radioFont);
    faceRadio->setChecked(true);
    faceRadio->setStyleSheet("color: #333;");
    mainLayout->addWidget(faceRadio);
    
    QRadioButton* wireRadio = new QRadioButton("Revolve Wire (Surface)", &dialog);
    wireRadio->setFont(radioFont);
    wireRadio->setStyleSheet("color: #333;");
    mainLayout->addWidget(wireRadio);
    
    QLabel* axisLabel = new QLabel("Rotation Axis:", &dialog);
    axisLabel->setFont(labelFont);
    axisLabel->setStyleSheet("font-weight: bold; color: #333;");
    mainLayout->addWidget(axisLabel);
    
    QComboBox* axisCombo = new QComboBox(&dialog);
    axisCombo->setFont(spinFont);
    axisCombo->addItem("X Axis");
    axisCombo->addItem("Y Axis");
    axisCombo->addItem("Z Axis");
    axisCombo->setFixedHeight(35);
    axisCombo->setStyleSheet("QComboBox { padding: 5px; border: 2px solid #ccc; border-radius: 6px; }");
    mainLayout->addWidget(axisCombo);
    
    QLabel* angleLabel = new QLabel("Rotation Angle (degrees):", &dialog);
    angleLabel->setFont(labelFont);
    angleLabel->setStyleSheet("font-weight: bold; color: #333;");
    mainLayout->addWidget(angleLabel);
    
    QDoubleSpinBox* angleSpin = new QDoubleSpinBox(&dialog);
    angleSpin->setFont(spinFont);
    angleSpin->setRange(0.1, 360.0);
    angleSpin->setValue(360.0);
    angleSpin->setDecimals(1);
    angleSpin->setFixedHeight(35);
    angleSpin->setStyleSheet("QDoubleSpinBox { padding: 5px; border: 2px solid #ccc; border-radius: 6px; }");
    mainLayout->addWidget(angleSpin);
    
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    QPushButton* okBtn = buttonBox->button(QDialogButtonBox::Ok);
    QPushButton* cancelBtn = buttonBox->button(QDialogButtonBox::Cancel);
    okBtn->setFont(QFont("Arial", 13));
    cancelBtn->setFont(QFont("Arial", 13));
    okBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 8px 20px; border: none; border-radius: 6px; } QPushButton:hover { background-color: #45a049; }");
    cancelBtn->setStyleSheet("QPushButton { background-color: #f44336; color: white; padding: 8px 20px; border: none; border-radius: 6px; } QPushButton:hover { background-color: #da190b; }");
    mainLayout->addWidget(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        bool revolveFace = faceRadio->isChecked();
        
        if (revolveFace) {
            TopoDS_Wire wire;
            if (basisShape.ShapeType() == TopAbs_WIRE) {
                wire = TopoDS::Wire(basisShape);
            } else if (basisShape.ShapeType() == TopAbs_EDGE) {
                TopoDS_Edge edge = TopoDS::Edge(basisShape);
                BRepBuilderAPI_MakeWire wireMaker(edge);
                wire = TopoDS::Wire(wireMaker.Shape());
            } else {
                QMessageBox::warning(this, "Revolve Error", "Cannot create face from this shape type.", QMessageBox::Ok);
                return;
            }
            
            BRepBuilderAPI_MakeFace faceMaker(wire);
            if (!faceMaker.IsDone()) {
                QMessageBox::warning(this, "Revolve Error", "Cannot revolve face because the sketch is not closed.", QMessageBox::Ok);
                return;
            }
            basisShape = faceMaker.Shape();
        }
        
        int axisIndex = axisCombo->currentIndex();
        gp_Dir axisDir;
        if (axisIndex == 0) axisDir = gp_Dir(1, 0, 0);
        else if (axisIndex == 1) axisDir = gp_Dir(0, 1, 0);
        else axisDir = gp_Dir(0, 0, 1);
        
        double angleDeg = angleSpin->value();
        double angleRad = angleDeg * M_PI / 180.0;
        
        gp_Ax1 axis(gp_Pnt(0, 0, 0), axisDir);
        
        Bnd_Box bbox;
        BRepBndLib::Add(basisShape, bbox);
        Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
        bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
        
        bool intersectsAxis = false;
        if (axisIndex == 0) {
            if (ymin <= 0.0 && ymax >= 0.0) {
                intersectsAxis = true;
            }
        } else if (axisIndex == 1) {
            if (xmin <= 0.0 && xmax >= 0.0) {
                intersectsAxis = true;
            }
        } else {
            if ((xmin <= 0.0 && xmax >= 0.0) && (ymin <= 0.0 && ymax >= 0.0)) {
                intersectsAxis = true;
            }
        }
        
        if (revolveFace && intersectsAxis) {
            QMessageBox::warning(this, "Revolve Error", "The sketch intersects the rotation axis. Cannot revolve face around intersecting axis.", QMessageBox::Ok);
            return;
        }
        
        TopoDS_Shape revolvedShape;
        if (fabs(angleDeg - 360.0) < 0.1) {
            revolvedShape = BRepPrimAPI_MakeRevol(basisShape, axis).Shape();
        } else {
            revolvedShape = BRepPrimAPI_MakeRevol(basisShape, axis, angleRad).Shape();
        }
        
        if (!revolvedShape.IsNull()) {
            Handle(AIS_Shape) aisShape = new AIS_Shape(revolvedShape);
            aisShape->SetColor(Quantity_NOC_YELLOW);
            aisShape->SetDisplayMode(AIS_Shaded);
            myContext->Display(aisShape, Standard_True);
            
            if (m_commandManager) {
                m_commandManager->executeCommand(new DisplayShapeCommand(myContext, aisShape, "Revolve"));
            }
            
            emit shapeCreated(aisShape, ShapeType::Revolve);
        }
    }
}

void OccView::handlePrimitiveCreation(QMouseEvent* theEvent)
{
    if (myCurrentMode < CurAction3d_Primitive_CreateBox ||
        myCurrentMode > CurAction3d_Primitive_CreateCone) {
        return;
    }

    if (theEvent->button() == Qt::LeftButton) {
        gp_Pnt clickedPoint = convertScreenToWorld(theEvent->pos());
        clickedPoint.SetZ(0.0);

        switch (myCurrentMode) {
        case CurAction3d_Primitive_CreateBox:
            handleBoxCreation(clickedPoint);
            break;
        case CurAction3d_Primitive_CreateSphere:
            handleSphereCreation(clickedPoint);
            break;
        case CurAction3d_Primitive_CreateCylinder:
            handleCylinderCreation(clickedPoint);
            break;
        case CurAction3d_Primitive_CreateCone:
            handleConeCreation(clickedPoint);
            break;
        }
    } else if (theEvent->button() == Qt::RightButton) {
        stopPrimitiveMode();
    }
}

void OccView::handlePrimitiveMouseMove(QMouseEvent* theEvent)
{
    if (myCurrentMode < CurAction3d_Primitive_CreateBox ||
        myCurrentMode > CurAction3d_Primitive_CreateCone) {
        return;
    }

    gp_Pnt mousePoint = convertScreenToWorld(theEvent->pos());
    mousePoint.SetZ(0.0);

    switch (myCurrentMode) {
    case CurAction3d_Primitive_CreateBox:
        updateBoxPreview(mousePoint);
        break;
    case CurAction3d_Primitive_CreateSphere:
        updateSpherePreview(mousePoint);
        break;
    case CurAction3d_Primitive_CreateCylinder:
        updateCylinderPreview(mousePoint);
        break;
    case CurAction3d_Primitive_CreateCone:
        updateConePreview(mousePoint);
        break;
    }
}

void OccView::handleBoxCreation(const gp_Pnt& clickedPoint)
{
    switch (m_primitivePhase) {
    case PrimitivePhase_Start:
        m_primitivePoint1 = clickedPoint;
        m_primitivePhase = PrimitivePhase_Base;
        break;
    case PrimitivePhase_Base: {
        m_primitivePoint2 = clickedPoint;
        
        double dx = fabs(m_primitivePoint2.X() - m_primitivePoint1.X());
        double dy = fabs(m_primitivePoint2.Y() - m_primitivePoint1.Y());
        
        if (dx < 0.001) dx = 0.001;
        if (dy < 0.001) dy = 0.001;

        QInputDialog dialog(this);
        dialog.setWindowTitle("Enter Box Height");
        dialog.setLabelText(QString("Width: %1, Depth: %2\nEnter Height:").arg(dx, 0, 'f', 3).arg(dy, 0, 'f', 3));
        dialog.setDoubleRange(-1000.0, 1000.0);
        dialog.setDoubleValue(std::max(dx, dy));
        dialog.setDoubleDecimals(3);
        dialog.setStyleSheet("QInputDialog { font-size: 14px; } QLabel { font-size: 14px; font-weight: bold; } QDoubleSpinBox { font-size: 14px; }");
        
        if (dialog.exec() == QDialog::Accepted) {
            double dz = dialog.doubleValue();
            double absDz = fabs(dz);
            if (absDz < 0.001) absDz = 0.001;
            
            double x = std::min(m_primitivePoint1.X(), m_primitivePoint2.X());
            double y = std::min(m_primitivePoint1.Y(), m_primitivePoint2.Y());
            double z = (dz < 0) ? dz : 0.0;

            TopoDS_Shape box = BRepPrimAPI_MakeBox(gp_Pnt(x, y, z), dx, dy, absDz).Shape();
            createPrimitiveShape(box, ShapeType::Box);
        }
        
        stopPrimitiveMode();
        break;
    }
    case PrimitivePhase_Height:
        break;
    }
}

void OccView::updateBoxPreview(const gp_Pnt& mousePoint)
{
    if (m_primitivePhase == PrimitivePhase_Base) {
        double dx = fabs(mousePoint.X() - m_primitivePoint1.X());
        double dy = fabs(mousePoint.Y() - m_primitivePoint1.Y());
        
        if (dx < 0.001) dx = 0.001;
        if (dy < 0.001) dy = 0.001;

        double x = std::min(m_primitivePoint1.X(), mousePoint.X());
        double y = std::min(m_primitivePoint1.Y(), mousePoint.Y());

        if (!m_previewShape.IsNull()) {
            myContext->Remove(m_previewShape, Standard_True);
        }
        
        TopoDS_Shape box = BRepPrimAPI_MakeBox(gp_Pnt(x, y, 0), dx, dy, 0.001).Shape();
        m_previewShape = new AIS_Shape(box);
        m_previewShape->SetColor(Quantity_NOC_CYAN1);
        m_previewShape->SetTransparency(0.5);
        myContext->Display(m_previewShape, Standard_True);
        myContext->UpdateCurrentViewer();
        myView->Redraw();
    } else if (m_primitivePhase == PrimitivePhase_Height) {
        double dx = fabs(m_primitivePoint2.X() - m_primitivePoint1.X());
        double dy = fabs(m_primitivePoint2.Y() - m_primitivePoint1.Y());
        double dz = fabs(mousePoint.Z());
        
        if (dx < 0.001) dx = 0.001;
        if (dy < 0.001) dy = 0.001;
        if (dz < 0.001) dz = 0.001;

        double x = std::min(m_primitivePoint1.X(), m_primitivePoint2.X());
        double y = std::min(m_primitivePoint1.Y(), m_primitivePoint2.Y());
        double z = 0.0;

        if (!m_previewShape.IsNull()) {
            myContext->Remove(m_previewShape, Standard_True);
        }
        
        TopoDS_Shape box = BRepPrimAPI_MakeBox(gp_Pnt(x, y, z), dx, dy, dz).Shape();
        m_previewShape = new AIS_Shape(box);
        m_previewShape->SetColor(Quantity_NOC_CYAN1);
        m_previewShape->SetTransparency(0.5);
        myContext->Display(m_previewShape, Standard_True);
        myContext->UpdateCurrentViewer();
        myView->Redraw();
    }
}

void OccView::handleSphereCreation(const gp_Pnt& clickedPoint)
{
    switch (m_primitivePhase) {
    case PrimitivePhase_Start:
        m_primitivePoint1 = clickedPoint;
        m_primitivePhase = PrimitivePhase_Base;
        break;
    case PrimitivePhase_Base: {
        double radius = m_primitivePoint1.Distance(clickedPoint);
        if (radius < 0.001) radius = 0.001;
        
        TopoDS_Shape sphere = BRepPrimAPI_MakeSphere(m_primitivePoint1, radius).Shape();
        createPrimitiveShape(sphere, ShapeType::Sphere);
        stopPrimitiveMode();
        break;
    }
    }
}

void OccView::updateSpherePreview(const gp_Pnt& mousePoint)
{
    if (m_primitivePhase == PrimitivePhase_Base) {
        double radius = m_primitivePoint1.Distance(mousePoint);
        if (radius < 0.001) radius = 0.001;

        if (!m_previewShape.IsNull()) {
            myContext->Remove(m_previewShape, Standard_True);
        }
        
        TopoDS_Shape sphere = BRepPrimAPI_MakeSphere(m_primitivePoint1, radius).Shape();
        m_previewShape = new AIS_Shape(sphere);
        m_previewShape->SetColor(Quantity_NOC_CYAN1);
        m_previewShape->SetTransparency(0.5);
        myContext->Display(m_previewShape, Standard_True);
        myContext->UpdateCurrentViewer();
        myView->Redraw();
    }
}

void OccView::handleCylinderCreation(const gp_Pnt& clickedPoint)
{
    switch (m_primitivePhase) {
    case PrimitivePhase_Start:
        m_primitivePoint1 = clickedPoint;
        m_primitivePhase = PrimitivePhase_Base;
        break;
    case PrimitivePhase_Base: {
        m_primitiveRadius1 = m_primitivePoint1.Distance(clickedPoint);
        if (m_primitiveRadius1 < 0.001) m_primitiveRadius1 = 0.001;

        QInputDialog dialog(this);
        dialog.setWindowTitle("Enter Cylinder Height");
        dialog.setLabelText(QString("Radius: %1\nEnter Height:").arg(m_primitiveRadius1, 0, 'f', 3));
        dialog.setDoubleRange(-1000.0, 1000.0);
        dialog.setDoubleValue(m_primitiveRadius1 * 2);
        dialog.setDoubleDecimals(3);
        dialog.setStyleSheet("QInputDialog { font-size: 14px; } QLabel { font-size: 14px; font-weight: bold; } QDoubleSpinBox { font-size: 14px; }");
        
        if (dialog.exec() == QDialog::Accepted) {
            double height = dialog.doubleValue();
            double absHeight = fabs(height);
            if (absHeight < 0.001) absHeight = 0.001;
            
            TopoDS_Shape cylinder = BRepPrimAPI_MakeCylinder(m_primitiveRadius1, absHeight).Shape();
            double zOffset = (height < 0) ? height : 0.0;
            gp_Trsf trsf;
            trsf.SetTranslation(gp_Vec(m_primitivePoint1.X(), m_primitivePoint1.Y(), zOffset));
            cylinder = BRepBuilderAPI_Transform(cylinder, trsf, Standard_True).Shape();
            createPrimitiveShape(cylinder, ShapeType::Cylinder);
        }
        
        stopPrimitiveMode();
        break;
    }
    case PrimitivePhase_Height:
        break;
    }
}

void OccView::updateCylinderPreview(const gp_Pnt& mousePoint)
{
    if (m_primitivePhase == PrimitivePhase_Base) {
        double radius = m_primitivePoint1.Distance(mousePoint);
        if (radius < 0.001) radius = 0.001;

        if (!m_previewShape.IsNull()) {
            myContext->Remove(m_previewShape, Standard_True);
        }
        
        TopoDS_Shape cylinder = BRepPrimAPI_MakeCylinder(radius, 0.001).Shape();
        gp_Trsf trsf;
        trsf.SetTranslation(gp_Vec(m_primitivePoint1.X(), m_primitivePoint1.Y(), 0));
        cylinder = BRepBuilderAPI_Transform(cylinder, trsf, Standard_True).Shape();
        
        m_previewShape = new AIS_Shape(cylinder);
        m_previewShape->SetColor(Quantity_NOC_CYAN1);
        m_previewShape->SetTransparency(0.5);
        myContext->Display(m_previewShape, Standard_True);
        myContext->UpdateCurrentViewer();
        myView->Redraw();
    } else if (m_primitivePhase == PrimitivePhase_Height) {
        if (m_primitiveRadius1 < 0.001) return;
        double height = fabs(mousePoint.Z());
        if (height < 0.001) height = 0.001;

        if (!m_previewShape.IsNull()) {
            myContext->Remove(m_previewShape, Standard_True);
        }
        
        TopoDS_Shape cylinder = BRepPrimAPI_MakeCylinder(m_primitiveRadius1, height).Shape();
        gp_Trsf trsf;
        trsf.SetTranslation(gp_Vec(m_primitivePoint1.X(), m_primitivePoint1.Y(), 0));
        cylinder = BRepBuilderAPI_Transform(cylinder, trsf, Standard_True).Shape();
        
        m_previewShape = new AIS_Shape(cylinder);
        m_previewShape->SetColor(Quantity_NOC_CYAN1);
        m_previewShape->SetTransparency(0.5);
        myContext->Display(m_previewShape, Standard_True);
        myContext->UpdateCurrentViewer();
        myView->Redraw();
    }
}

void OccView::handleConeCreation(const gp_Pnt& clickedPoint)
{
    switch (m_primitivePhase) {
    case PrimitivePhase_Start:
        m_primitivePoint1 = clickedPoint;
        m_primitivePhase = PrimitivePhase_Base;
        break;
    case PrimitivePhase_Base: {
        m_primitiveRadius1 = m_primitivePoint1.Distance(clickedPoint);
        if (m_primitiveRadius1 < 0.001) m_primitiveRadius1 = 0.001;

        QInputDialog dialog(this);
        dialog.setWindowTitle("Enter Cone Height");
        dialog.setLabelText(QString("Base Radius: %1\nEnter Height:").arg(m_primitiveRadius1, 0, 'f', 3));
        dialog.setDoubleRange(-1000.0, 1000.0);
        dialog.setDoubleValue(m_primitiveRadius1 * 2);
        dialog.setDoubleDecimals(3);
        dialog.setStyleSheet("QInputDialog { font-size: 14px; } QLabel { font-size: 14px; font-weight: bold; } QDoubleSpinBox { font-size: 14px; }");
        
        if (dialog.exec() == QDialog::Accepted) {
            double height = dialog.doubleValue();
            double absHeight = fabs(height);
            if (absHeight < 0.001) absHeight = 0.001;
            
            TopoDS_Shape cone = BRepPrimAPI_MakeCone(m_primitiveRadius1, 0.001, absHeight).Shape();
            double zOffset = (height < 0) ? height : 0.0;
            gp_Trsf trsf;
            trsf.SetTranslation(gp_Vec(m_primitivePoint1.X(), m_primitivePoint1.Y(), zOffset));
            cone = BRepBuilderAPI_Transform(cone, trsf, Standard_True).Shape();
            createPrimitiveShape(cone, ShapeType::Cone);
        }
        
        stopPrimitiveMode();
        break;
    }
    }
}

void OccView::updateConePreview(const gp_Pnt& mousePoint)
{
    if (m_primitivePhase == PrimitivePhase_Base) {
        double radius = m_primitivePoint1.Distance(mousePoint);
        if (radius < 0.001) radius = 0.001;

        if (!m_previewShape.IsNull()) {
            myContext->Remove(m_previewShape, Standard_True);
        }
        
        TopoDS_Shape cone = BRepPrimAPI_MakeCone(radius, 0.001, 0.001).Shape();
        gp_Trsf trsf;
        trsf.SetTranslation(gp_Vec(m_primitivePoint1.X(), m_primitivePoint1.Y(), 0));
        cone = BRepBuilderAPI_Transform(cone, trsf, Standard_True).Shape();
        
        m_previewShape = new AIS_Shape(cone);
        m_previewShape->SetColor(Quantity_NOC_CYAN1);
        m_previewShape->SetTransparency(0.5);
        myContext->Display(m_previewShape, Standard_True);
        myContext->UpdateCurrentViewer();
        myView->Redraw();
    } else if (m_primitivePhase == PrimitivePhase_Height) {
        if (m_primitiveRadius1 < 0.001) return;
        double height = fabs(mousePoint.Z());
        if (height < 0.001) height = 0.001;

        if (!m_previewShape.IsNull()) {
            myContext->Remove(m_previewShape, Standard_True);
        }
        
        TopoDS_Shape cone = BRepPrimAPI_MakeCone(m_primitiveRadius1, 0.001, height).Shape();
        gp_Trsf trsf;
        trsf.SetTranslation(gp_Vec(m_primitivePoint1.X(), m_primitivePoint1.Y(), 0));
        cone = BRepBuilderAPI_Transform(cone, trsf, Standard_True).Shape();
        
        m_previewShape = new AIS_Shape(cone);
        m_previewShape->SetColor(Quantity_NOC_CYAN1);
        m_previewShape->SetTransparency(0.5);
        myContext->Display(m_previewShape, Standard_True);
        myContext->UpdateCurrentViewer();
        myView->Redraw();
    }
}

void OccView::createPrimitiveShape(const TopoDS_Shape& shape, ShapeType type)
{
    if (!m_previewShape.IsNull()) {
        myContext->Remove(m_previewShape, Standard_True);
        m_previewShape.Nullify();
    }

    Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
    aisShape->SetColor(Quantity_NOC_YELLOW);
    aisShape->SetDisplayMode(AIS_Shaded);
    myContext->Display(aisShape, Standard_True);

    if (m_commandManager) {
        m_commandManager->executeCommand(new DisplayShapeCommand(myContext, aisShape, "Create Primitive"));
    }

    myContext->ClearSelected(Standard_False);
    myContext->UpdateCurrentViewer();
    myView->Redraw();

    emit shapeCreated(aisShape, type);
}