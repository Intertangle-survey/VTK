/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkVertex.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkVertex.h"

#include "vtkMath.h"
#include "vtkCellArray.h"
#include "vtkPointLocator.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkCellData.h"

vtkCxxRevisionMacro(vtkVertex, "1.56");
vtkStandardNewMacro(vtkVertex);

// Construct the vertex with a single point.
vtkVertex::vtkVertex()
{
  int i;
  
  this->Points->SetNumberOfPoints(1);
  this->PointIds->SetNumberOfIds(1);
  for (i = 0; i < 1; i++)
    {
    this->Points->SetPoint(i, 0.0, 0.0, 0.0);
    }
  for (i = 0; i < 1; i++)
    {
    this->PointIds->SetId(i,0);
    }
}

// Make a new vtkVertex object with the same information as this object.
vtkCell *vtkVertex::MakeObject()
{
  vtkCell *cell = vtkVertex::New();
  cell->DeepCopy(this);
  return cell;
}

int vtkVertex::EvaluatePosition(float x[3], float* closestPoint,
                                int& subId, float pcoords[3], 
                                float& dist2, float *weights)
{
  float *X;

  subId = 0;
  pcoords[1] = pcoords[2] = 0.0;

  X = this->Points->GetPoint(0);
  if (closestPoint)
    {
    closestPoint[0] = X[0]; closestPoint[1] = X[1]; closestPoint[2] = X[2];
    }

  dist2 = vtkMath::Distance2BetweenPoints(X,x);
  weights[0] = 1.0;

  if (dist2 == 0.0)
    {
    pcoords[0] = 0.0;
    return 1;
    }
  else
    {
    pcoords[0] = -10.0;
    return 0;
    }
}

void vtkVertex::EvaluateLocation(int& vtkNotUsed(subId), 
                                 float vtkNotUsed(pcoords)[3], float x[3],
                                 float *weights)
{
  float *X = this->Points->GetPoint(0);
  x[0] = X[0];
  x[1] = X[1];
  x[2] = X[2];

  weights[0] = 1.0;
}

// Given parametric coordinates of a point, return the closest cell boundary,
// and whether the point is inside or outside of the cell. The cell boundary 
// is defined by a list of points (pts) that specify a vertex (1D cell). 
// If the return value of the method is != 0, then the point is inside the cell.
int vtkVertex::CellBoundary(int vtkNotUsed(subId), float pcoords[3], 
                            vtkIdList *pts)
{

  pts->SetNumberOfIds(1);
  pts->SetId(0,this->PointIds->GetId(0));

  if ( pcoords[0] != 0.0 )  
    {
    return 0;
    }
  else
    {
    return 1;
    }

}

// Generate contouring primitives. The scalar list cellScalars are
// scalar values at each cell point. The point locator is essentially a 
// points list that merges points as they are inserted (i.e., prevents 
// duplicates). 
void vtkVertex::Contour(float value, vtkDataArray *cellScalars, 
                        vtkPointLocator *locator,
                        vtkCellArray *verts, 
                        vtkCellArray *vtkNotUsed(lines), 
                        vtkCellArray *vtkNotUsed(polys), 
                        vtkPointData *inPd, vtkPointData *outPd,
                        vtkCellData *inCd, vtkIdType cellId, vtkCellData *outCd)
{
  if ( value == cellScalars->GetComponent(0,0) )
    {
    int newCellId;
    vtkIdType pts[1];
    pts[0] = locator->InsertNextPoint(this->Points->GetPoint(0));
    if ( outPd )
      {   
      outPd->CopyData(inPd,this->PointIds->GetId(0),pts[0]);
      }
    newCellId = verts->InsertNextCell(1,pts);
    outCd->CopyData(inCd,cellId,newCellId);
    }
}

// Intersect with a ray. Return parametric coordinates (both line and cell)
// and global intersection coordinates, given ray definition and tolerance. 
// The method returns non-zero value if intersection occurs.
int vtkVertex::IntersectWithLine(float p1[3], float p2[3], float tol, float& t,
                                float x[3], float pcoords[3], int& subId)
{
  int i;
  float *X, ray[3], rayFactor, projXYZ[3];

  subId = 0;
  pcoords[1] = pcoords[2] = 0.0;

  X = this->Points->GetPoint(0);

  for (i=0; i<3; i++)
    {
    ray[i] = p2[i] - p1[i];
    }
  if (( rayFactor = vtkMath::Dot(ray,ray)) == 0.0 )
    {
    return 0;
    }
  //
  //  Project each point onto ray. Determine whether point is within tolerance.
  //
  t = (ray[0]*(X[0]-p1[0]) + ray[1]*(X[1]-p1[1]) + ray[2]*(X[2]-p1[2]))
      / rayFactor;

  if ( t >= 0.0 && t <= 1.0 )
    {
    for (i=0; i<3; i++) 
      {
      projXYZ[i] = p1[i] + t*ray[i];
      if ( fabs(X[i]-projXYZ[i]) > tol )
        {
        break;
        }
      }

    if ( i > 2 ) // within tolerance 
      {
      pcoords[0] = 0.0;
      x[0] = X[0]; x[1] = X[1]; x[2] = X[2]; 
      return 1;
      }
    }

  pcoords[0] = -10.0;
  return 0;
}

// Triangulate the vertex. This method fills pts and ptIds with information
// from the only point in the vertex.
int vtkVertex::Triangulate(int vtkNotUsed(index),vtkIdList *ptIds,
                           vtkPoints *pts)
{
  pts->Reset();
  ptIds->Reset();
  pts->InsertPoint(0,this->Points->GetPoint(0));
  ptIds->InsertId(0,this->PointIds->GetId(0));

  return 1;
}

// Get the derivative of the vertex. Returns (0.0, 0.0, 0.0) for all 
// dimensions.
void vtkVertex::Derivatives(int vtkNotUsed(subId), 
                            float vtkNotUsed(pcoords)[3], 
                            float *vtkNotUsed(values), 
                            int dim, float *derivs)
{
  int i, idx;

  for (i=0; i<dim; i++)
    {
    idx = i*dim;
    derivs[idx] = 0.0;
    derivs[idx+1] = 0.0;
    derivs[idx+2] = 0.0;
    }
}

void vtkVertex::Clip(float value, vtkDataArray *cellScalars, 
                     vtkPointLocator *locator, vtkCellArray *verts,
                     vtkPointData *inPd, vtkPointData *outPd,
                     vtkCellData *inCd, vtkIdType cellId, vtkCellData *outCd,
                     int insideOut)
{
  float s, *x;
  int newCellId;
  vtkIdType pts[1];
  
  s = cellScalars->GetComponent(0,0);

  if ( ( !insideOut && s > value) || (insideOut && s <= value) )
    {
    x = this->Points->GetPoint(0);
    if ( locator->InsertUniquePoint(x, pts[0]) )
      {
      outPd->CopyData(inPd,this->PointIds->GetId(0),pts[0]);
      }
    newCellId = verts->InsertNextCell(1,pts);
    outCd->CopyData(inCd,cellId,newCellId);
    }

}
//
// Compute interpolation functions
//
void vtkVertex::InterpolationFunctions(float vtkNotUsed(pcoords)[3], float weights[1])
{
  weights[0] = 1.0;
}
