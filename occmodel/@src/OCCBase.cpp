// Gmsh - Copyright (C) 1997-2011 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.
#include "OCCModel.h"

int OCCBase::transform(DVec mat)
{
    try {
        TopoDS_Shape shape = this->getShape();
        gp_Trsf trans;
        trans.SetValues(
            mat[0], mat[1], mat[2], mat[3], 
            mat[4], mat[5], mat[6], mat[7], 
            mat[8], mat[9], mat[10], mat[11], 
            0.00001,0.00001
        );
        // Check if scaling is non-uniform
        double scaleTol = mat[0]*mat[5]*mat[10] - 1.0;
        if (scaleTol > 1e-6) {
            BRepBuilderAPI_GTransform aTrans(shape, trans, Standard_False);
            aTrans.Build();
            aTrans.Check();
            this->setShape(aTrans.Shape());
        } else {
            BRepBuilderAPI_Transform aTrans(shape, trans, Standard_False);
            aTrans.Build();
            aTrans.Check();
            this->setShape(aTrans.Shape());
        }
    } catch(Standard_Failure &err) {
        return 1;
    }
    return 0;
}

int OCCBase::translate(DVec delta)
{
    try {
        TopoDS_Shape shape = this->getShape();
        gp_Trsf trans;
        trans.SetTranslation(gp_Pnt(0,0,0), gp_Pnt(delta[0],delta[1],delta[2]));
        TopLoc_Location loc = shape.Location();
        gp_Trsf placement = loc.Transformation();
        trans = placement * trans;
        BRepBuilderAPI_Transform aTrans(shape, trans, Standard_False);
        aTrans.Build();
        aTrans.Check();
        this->setShape(aTrans.Shape());
    } catch(Standard_Failure &err) {
        return 1;
    }
    return 0;
}

int OCCBase::rotate(DVec p1, DVec p2, double angle)
{
    try {
        TopoDS_Shape shape = this->getShape();
        gp_Trsf trans;
        gp_Vec dir(gp_Pnt(p1[0], p1[1], p1[2]), gp_Pnt(p2[0], p2[1], p2[2]));
        gp_Ax1 axis(gp_Pnt(p1[0], p1[1], p1[2]), dir);
        trans.SetRotation(axis, angle);
        TopLoc_Location loc = shape.Location();
        gp_Trsf placement = loc.Transformation();
        trans = placement * trans;
        BRepBuilderAPI_Transform aTrans(shape, trans, Standard_False);
        this->setShape(aTrans.Shape());
    } catch(Standard_Failure &err) {
        return 1;
    }
    return 0;
}

int OCCBase::scale(DVec pnt, double scale)
{
    try {
        TopoDS_Shape shape = this->getShape();
        gp_Trsf trans;
        trans.SetScale(gp_Pnt(pnt[0],pnt[1],pnt[2]), scale);
        TopLoc_Location loc = shape.Location();
        gp_Trsf placement = loc.Transformation();
        trans = placement * trans;
        BRepBuilderAPI_Transform aTrans(shape, trans, Standard_True);
        this->setShape(aTrans.Shape());
    } catch(Standard_Failure &err) {
        return 1;
    }
    return 0;
}

int OCCBase::mirror(DVec pnt, DVec nor)
{
    try {
        TopoDS_Shape shape = this->getShape();
        gp_Ax2 ax2(gp_Pnt(pnt[0],pnt[1],pnt[2]), gp_Dir(nor[0],nor[1],nor[2]));
        gp_Trsf trans;
        trans.SetMirror(ax2);
        TopLoc_Location loc = shape.Location();
        gp_Trsf placement = loc.Transformation();
        trans = placement * trans;
        BRepBuilderAPI_Transform aTrans(shape, trans, Standard_False);
        this->setShape(aTrans.Shape());
    } catch(Standard_Failure &err) {
        return 1;
    }
    return 0;
}

int OCCBase::findPlane(double *origin, double *normal, double tolerance = 1e-6)
{
    try {
        TopoDS_Shape shape = this->getShape();
        BRepBuilderAPI_FindPlane FP(shape, tolerance);
        if(!FP.Found()) return 1;
        const Handle_Geom_Plane plane = FP.Plane();
        const gp_Ax1 axis = plane->Axis();
        
        const gp_Pnt loc = axis.Location();
        origin[0] = loc.X();
        origin[1] = loc.Y();
        origin[2] = loc.Z();
        
        const gp_Dir dir = axis.Direction();
        normal[0] = dir.X();
        normal[1] = dir.Y();
        normal[2] = dir.Z();
        
    } catch(Standard_Failure &err) {
        return 1;
    }
    return 0;   
}
DVec OCCBase::boundingBox(double tolerance = 1e-12)
{
    DVec ret;
    try {
        TopoDS_Shape shape = this->getShape();
        Bnd_Box aBox;
        BRepBndLib::Add(shape, aBox);
        aBox.SetGap(tolerance);
        Standard_Real aXmin, aYmin, aZmin;
        Standard_Real aXmax, aYmax, aZmax;
        aBox.Get(aXmin, aYmin, aZmin, aXmax, aYmax, aZmax);
        ret.push_back(aXmin);
        ret.push_back(aYmin);
        ret.push_back(aZmin);
        ret.push_back(aXmax);
        ret.push_back(aYmax);
        ret.push_back(aZmax);
    } catch(Standard_Failure &err) {
        ret.push_back(0.0);
        ret.push_back(0.0);
        ret.push_back(0.0);
        ret.push_back(0.0);
        ret.push_back(0.0);
        ret.push_back(0.0);
    }
    return ret;
}