#pragma once

#include <QString>
#include <QStack>
#include <QList>

#include <AIS_InteractiveObject.hxx>
#include <AIS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <gp_Pnt.hxx>
#include <gp_Circ.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <Quantity_Color.hxx>

// Command base class
class Command
{
public:
    virtual ~Command() {}
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual QString description() const = 0;
};

// Draw Point Command
class DrawPointCommand : public Command
{
private:
    Handle(AIS_InteractiveContext) m_context;
    Handle(AIS_Shape) m_pointShape;
    gp_Pnt m_point;
    Quantity_Color m_color;
    bool m_executed;

public:
    DrawPointCommand(const Handle(AIS_InteractiveContext)& ctx, double x, double y, const Quantity_Color& color = Quantity_NOC_RED)
        : m_context(ctx), m_point(x, y, 0.0), m_color(color), m_executed(false)
    {
        TopoDS_Shape vertex = BRepBuilderAPI_MakeVertex(m_point).Shape();
        m_pointShape = new AIS_Shape(vertex);
        m_pointShape->SetColor(m_color);
    }

    void execute() override
    {
        if (!m_context.IsNull() && !m_pointShape.IsNull())
        {
            m_context->Display(m_pointShape, Standard_True);
            m_context->UpdateCurrentViewer();
            m_executed = true;
        }
    }

    void undo() override
    {
        if (!m_context.IsNull() && !m_pointShape.IsNull() && m_executed)
        {
            m_context->Erase(m_pointShape, Standard_False);
            m_context->UpdateCurrentViewer();
            m_executed = false;
        }
    }

    QString description() const override { return "Draw Point"; }

    Handle(AIS_Shape) getShape() const { return m_pointShape; }
};

// Draw Line Command
class DrawLineCommand : public Command
{
private:
    Handle(AIS_InteractiveContext) m_context;
    Handle(AIS_Shape) m_lineShape;
    gp_Pnt m_p1, m_p2;
    Quantity_Color m_color;
    bool m_executed;

public:
    DrawLineCommand(const Handle(AIS_InteractiveContext)& ctx, double x1, double y1, double x2, double y2, const Quantity_Color& color = Quantity_NOC_BLUE)
        : m_context(ctx), m_p1(x1, y1, 0.0), m_p2(x2, y2, 0.0), m_color(color), m_executed(false)
    {
        BRepBuilderAPI_MakeEdge edgeMaker(m_p1, m_p2);
        m_lineShape = new AIS_Shape(edgeMaker.Shape());
        m_lineShape->SetColor(m_color);
    }

    void execute() override
    {
        if (!m_context.IsNull() && !m_lineShape.IsNull())
        {
            m_context->Display(m_lineShape, Standard_True);
            m_context->UpdateCurrentViewer();
            m_executed = true;
        }
    }

    void undo() override
    {
        if (!m_context.IsNull() && !m_lineShape.IsNull() && m_executed)
        {
            m_context->Erase(m_lineShape, Standard_False);
            m_context->UpdateCurrentViewer();
            m_executed = false;
        }
    }

    QString description() const override { return "Draw Line"; }

    Handle(AIS_Shape) getShape() const { return m_lineShape; }
};

// Draw Circle Command
class DrawCircleCommand : public Command
{
private:
    Handle(AIS_InteractiveContext) m_context;
    Handle(AIS_Shape) m_circleShape;
    gp_Pnt m_center;
    double m_radius;
    Quantity_Color m_color;
    bool m_executed;

public:
    DrawCircleCommand(const Handle(AIS_InteractiveContext)& ctx, double cx, double cy, double radius, const Quantity_Color& color = Quantity_NOC_GREEN)
        : m_context(ctx), m_center(cx, cy, 0.0), m_radius(radius), m_color(color), m_executed(false)
    {
        gp_Circ circle(gp_Ax2(m_center, gp_Dir(0, 0, 1)), m_radius);
        BRepBuilderAPI_MakeEdge edgeMaker(circle);
        m_circleShape = new AIS_Shape(edgeMaker.Shape());
        m_circleShape->SetColor(m_color);
    }

    void execute() override
    {
        if (!m_context.IsNull() && !m_circleShape.IsNull())
        {
            m_context->Display(m_circleShape, Standard_True);
            m_context->UpdateCurrentViewer();
            m_executed = true;
        }
    }

    void undo() override
    {
        if (!m_context.IsNull() && !m_circleShape.IsNull() && m_executed)
        {
            m_context->Erase(m_circleShape, Standard_False);
            m_context->UpdateCurrentViewer();
            m_executed = false;
        }
    }

    QString description() const override { return "Draw Circle"; }

    Handle(AIS_Shape) getShape() const { return m_circleShape; }
};

// Delete Command
class DeleteCommand : public Command
{
private:
    Handle(AIS_InteractiveContext) m_context;
    QList<Handle(AIS_InteractiveObject)> m_deletedObjects;
    bool m_executed;

public:
    DeleteCommand(const Handle(AIS_InteractiveContext)& ctx, const QList<Handle(AIS_InteractiveObject)>& objects)
        : m_context(ctx), m_deletedObjects(objects), m_executed(false)
    {
    }

    void execute() override
    {
        if (!m_context.IsNull())
        {
            for (const Handle(AIS_InteractiveObject)& obj : m_deletedObjects)
            {
                if (m_context->IsDisplayed(obj))
                {
                    m_context->Erase(obj, Standard_False);
                }
            }
            m_context->UpdateCurrentViewer();
            m_executed = true;
        }
    }

    void undo() override
    {
        if (!m_context.IsNull() && m_executed)
        {
            for (const Handle(AIS_InteractiveObject)& obj : m_deletedObjects)
            {
                m_context->Display(obj, Standard_True);
            }
            m_context->UpdateCurrentViewer();
            m_executed = false;
        }
    }

    QString description() const override { return "Delete Objects"; }
};

// Display Shape Command - wraps an already-displayed shape for undo/redo
class DisplayShapeCommand : public Command
{
private:
    Handle(AIS_InteractiveContext) m_context;
    Handle(AIS_Shape) m_shape;
    QString m_desc;
    bool m_executed;

public:
    DisplayShapeCommand(const Handle(AIS_InteractiveContext)& ctx, const Handle(AIS_Shape)& shape, const QString& desc = "Draw Shape")
        : m_context(ctx), m_shape(shape), m_desc(desc), m_executed(true)
    {
    }

    void execute() override
    {
        if (!m_context.IsNull() && !m_shape.IsNull() && !m_executed)
        {
            m_context->Display(m_shape, Standard_True);
            m_context->UpdateCurrentViewer();
            m_executed = true;
        }
    }

    void undo() override
    {
        if (!m_context.IsNull() && !m_shape.IsNull() && m_executed)
        {
            m_context->Erase(m_shape, Standard_False);
            m_context->UpdateCurrentViewer();
            m_executed = false;
        }
    }

    QString description() const override { return m_desc; }
};

// Command Manager
class CommandManager
{
private:
    QStack<Command*> m_undoStack;
    QStack<Command*> m_redoStack;
    int m_maxDepth;

public:
    CommandManager(int maxDepth = 50) : m_maxDepth(maxDepth) {}

    ~CommandManager()
    {
        clear();
    }

    void executeCommand(Command* cmd)
    {
        if (cmd)
        {
            cmd->execute();
            m_undoStack.push(cmd);

            while (m_undoStack.size() > m_maxDepth)
            {
                Command* oldCmd = m_undoStack.takeFirst();
                delete oldCmd;
            }

            clearRedoStack();
        }
    }

    bool canUndo() const
    {
        return !m_undoStack.isEmpty();
    }

    bool canRedo() const
    {
        return !m_redoStack.isEmpty();
    }

    void undo()
    {
        if (!m_undoStack.isEmpty())
        {
            Command* cmd = m_undoStack.pop();
            cmd->undo();
            m_redoStack.push(cmd);
        }
    }

    void redo()
    {
        if (!m_redoStack.isEmpty())
        {
            Command* cmd = m_redoStack.pop();
            cmd->execute();
            m_undoStack.push(cmd);
        }
    }

    QString undoDescription() const
    {
        if (m_undoStack.isEmpty())
            return "";
        return m_undoStack.top()->description();
    }

    QString redoDescription() const
    {
        if (m_redoStack.isEmpty())
            return "";
        return m_redoStack.top()->description();
    }

    void clear()
    {
        clearUndoStack();
        clearRedoStack();
    }

private:
    void clearUndoStack()
    {
        while (!m_undoStack.isEmpty())
        {
            delete m_undoStack.pop();
        }
    }

    void clearRedoStack()
    {
        while (!m_redoStack.isEmpty())
        {
            delete m_redoStack.pop();
        }
    }
};

