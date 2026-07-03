#include "GeometryModel.h"
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <IGESControl_Reader.hxx>
#include <IGESControl_Writer.hxx>
#include <BRepTools.hxx>
#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>

// Force recompile

bool GeometryModel::readStepFile(const std::string& fileName, std::string& errorMessage) {
    STEPControl_Reader reader;
    IFSelect_ReturnStatus status = reader.ReadFile(fileName.c_str());
    
    if (status != IFSelect_RetDone) {
        errorMessage = "Failed to read STEP file";
        return false;
    }
    
    reader.TransferRoots();
    TopoDS_Shape shape = reader.OneShape();
    
    if (shape.IsNull()) {
        errorMessage = "No shape found in STEP file";
        return false;
    }
    
    addShape(shape, ShapeType::Model);
    return true;
}

bool GeometryModel::readBrepFile(const std::string& fileName, std::string& errorMessage) {
    TopoDS_Shape shape;
    BRep_Builder builder;
    
    if (!BRepTools::Read(shape, fileName.c_str(), builder)) {
        errorMessage = "Failed to read BREP file";
        return false;
    }
    
    if (shape.IsNull()) {
        errorMessage = "No shape found in BREP file";
        return false;
    }
    
    addShape(shape, ShapeType::Model);
    return true;
}

bool GeometryModel::readIgesFile(const std::string& fileName, std::string& errorMessage) {
    IGESControl_Reader reader;
    IFSelect_ReturnStatus status = reader.ReadFile(fileName.c_str());
    
    if (status != IFSelect_RetDone) {
        errorMessage = "Failed to read IGES file";
        return false;
    }
    
    reader.TransferRoots();
    TopoDS_Shape shape = reader.OneShape();
    
    if (shape.IsNull()) {
        errorMessage = "No shape found in IGES file";
        return false;
    }
    
    addShape(shape, ShapeType::Model);
    return true;
}

bool GeometryModel::writeStepFile(const std::string& fileName, std::string& errorMessage) {
    if (m_shapes.empty()) {
        errorMessage = "No shapes to save";
        return false;
    }
    
    // Combine all shapes into a Compound
    TopoDS_Compound compound;
    BRep_Builder builder;
    builder.MakeCompound(compound);
    
    for (const auto& aisShape : m_shapes) {
        if (!aisShape.IsNull()) {
            builder.Add(compound, aisShape->Shape());
        }
    }
    
    STEPControl_Writer writer;
    writer.Transfer(compound, STEPControl_AsIs);
    
    IFSelect_ReturnStatus status = writer.Write(fileName.c_str());
    if (status != IFSelect_RetDone) {
        errorMessage = "Failed to write STEP file";
        return false;
    }
    
    return true;
}

bool GeometryModel::writeBrepFile(const std::string& fileName, std::string& errorMessage) {
    if (m_shapes.empty()) {
        errorMessage = "No shapes to save";
        return false;
    }
    
    // Combine all shapes into a Compound
    TopoDS_Compound compound;
    BRep_Builder builder;
    builder.MakeCompound(compound);
    
    for (const auto& aisShape : m_shapes) {
        if (!aisShape.IsNull()) {
            builder.Add(compound, aisShape->Shape());
        }
    }
    
    if (!BRepTools::Write(compound, fileName.c_str())) {
        errorMessage = "Failed to write BREP file";
        return false;
    }
    
    return true;
}

bool GeometryModel::writeIgesFile(const std::string& fileName, std::string& errorMessage) {
    if (m_shapes.empty()) {
        errorMessage = "No shapes to save";
        return false;
    }
    
    // Combine all shapes into a Compound
    TopoDS_Compound compound;
    BRep_Builder builder;
    builder.MakeCompound(compound);
    
    for (const auto& aisShape : m_shapes) {
        if (!aisShape.IsNull()) {
            builder.Add(compound, aisShape->Shape());
        }
    }
    
    IGESControl_Writer writer;
    writer.AddShape(compound);
    writer.ComputeModel();
    
    if (!writer.Write(fileName.c_str())) {
        errorMessage = "Failed to write IGES file";
        return false;
    }
    
    return true;
}