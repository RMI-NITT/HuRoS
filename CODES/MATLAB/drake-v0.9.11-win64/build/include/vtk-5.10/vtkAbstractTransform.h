/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkAbstractTransform.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkAbstractTransform - superclass for all geometric transformations
// .SECTION Description
// vtkAbstractTransform is the superclass for all VTK geometric 
// transformations.  The VTK transform hierarchy is split into two
// major branches: warp transformations and homogeneous (including linear)
// transformations.  The latter can be represented in terms of a 4x4
// transformation matrix, the former cannot.  
// <p>Transformations can be pipelined through two mechanisms:  
// <p>1) GetInverse() returns the pipelined
// inverse of a transformation i.e. if you modify the original transform,
// any transform previously returned by the GetInverse() method will
// automatically update itself according to the change.
// <p>2) You can do pipelined concatenation of transformations through 
// the vtkGeneralTransform class, the vtkPerspectiveTransform class,
// or the vtkTransform class. 
// .SECTION see also
// vtkGeneralTransform vtkWarpTransform vtkHomogeneousTransform
// vtkLinearTransform vtkIdentityTransform
// vtkTransformPolyDataFilter vtkTransformFilter vtkImageReslice
// vtkImplicitFunction


#ifndef __vtkAbstractTransform_h
#define __vtkAbstractTransform_h

#include "vtkObject.h"

class vtkDataArray;
class vtkMatrix4x4;
class vtkPoints;
class vtkSimpleCriticalSection;

class VTK_COMMON_EXPORT vtkAbstractTransform : public vtkObject
{
public:

  vtkTypeMacro(vtkAbstractTransform,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Apply the transformation to a coordinate.  You can use the same 
  // array to store both the input and output point.
  void TransformPoint(const float in[3], float out[3]) {
    this->Update(); this->InternalTransformPoint(in,out); };

  // Description:
  // Apply the transformation to a double-precision coordinate.  
  // You can use the same array to store both the input and output point.
  void TransformPoint(const double in[3], double out[3]) {
    this->Update(); this->InternalTransformPoint(in,out); };

  // Description:
  // Apply the transformation to a double-precision coordinate.  
  // Use this if you are programming in Python, tcl or Java.
  double *TransformPoint(double x, double y, double z) {
    return this->TransformDoublePoint(x,y,z); }
  double *TransformPoint(const double point[3]) {
    return this->TransformPoint(point[0],point[1],point[2]); };

  // Description:
  // Apply the transformation to an (x,y,z) coordinate.
  // Use this if you are programming in Python, tcl or Java.
  float *TransformFloatPoint(float x, float y, float z) {
      this->InternalFloatPoint[0] = x;
      this->InternalFloatPoint[1] = y;
      this->InternalFloatPoint[2] = z;
      this->TransformPoint(this->InternalFloatPoint,this->InternalFloatPoint);
      return this->InternalFloatPoint; };
  float *TransformFloatPoint(const float point[3]) {
    return this->TransformFloatPoint(point[0],point[1],point[2]); };

  // Description:
  // Apply the transformation to a double-precision (x,y,z) coordinate.
  // Use this if you are programming in Python, tcl or Java.
  double *TransformDoublePoint(double x, double y, double z) {
    this->InternalDoublePoint[0] = x;
    this->InternalDoublePoint[1] = y;
    this->InternalDoublePoint[2] = z;
    this->TransformPoint(this->InternalDoublePoint,this->InternalDoublePoint);
    return this->InternalDoublePoint; };
  double *TransformDoublePoint(const double point[3]) {
    return this->TransformDoublePoint(point[0],point[1],point[2]); };

  // Description:
  // Apply the transformation to a normal at the specified vertex.  If the
  // transformation is a vtkLinearTransform, you can use TransformNormal()
  // instead.
  void TransformNormalAtPoint(const float point[3], const float in[3],
                              float out[3]);
  void TransformNormalAtPoint(const double point[3], const double in[3],
                              double out[3]);

  double *TransformNormalAtPoint(const double point[3], 
                                 const double normal[3]) {
    this->TransformNormalAtPoint(point,normal,this->InternalDoublePoint);
    return this->InternalDoublePoint; };

  // Description:
  // Apply the transformation to a double-precision normal at the specified
  // vertex.  If the transformation is a vtkLinearTransform, you can use
  // TransformDoubleNormal() instead.
  double *TransformDoubleNormalAtPoint(const double point[3],
                                       const double normal[3]) {
    this->TransformNormalAtPoint(point,normal,this->InternalDoublePoint);
    return this->InternalDoublePoint; };
  
  // Description:
  // Apply the transformation to a single-precision normal at the specified
  // vertex.  If the transformation is a vtkLinearTransform, you can use
  // TransformFloatNormal() instead.
  float *TransformFloatNormalAtPoint(const float point[3],
                                     const float normal[3]) {
    this->TransformNormalAtPoint(point,normal,this->InternalFloatPoint);
    return this->InternalFloatPoint; };

  // Description:
  // Apply the transformation to a vector at the specified vertex.  If the
  // transformation is a vtkLinearTransform, you can use TransformVector()
  // instead.
  void TransformVectorAtPoint(const float point[3], const float in[3],
                              float out[3]);
  void TransformVectorAtPoint(const double point[3], const double in[3],
                              double out[3]);

  double *TransformVectorAtPoint(const double point[3], 
                                 const double vector[3]) {
    this->TransformVectorAtPoint(point,vector,this->InternalDoublePoint);
    return this->InternalDoublePoint; };

  // Description:
  // Apply the transformation to a double-precision vector at the specified
  // vertex.  If the transformation is a vtkLinearTransform, you can use
  // TransformDoubleVector() instead.
  double *TransformDoubleVectorAtPoint(const double point[3],
                                       const double vector[3]) {
    this->TransformVectorAtPoint(point,vector,this->InternalDoublePoint);
    return this->InternalDoublePoint; };
  
  // Description:
  // Apply the transformation to a single-precision vector at the specified
  // vertex.  If the transformation is a vtkLinearTransform, you can use
  // TransformFloatVector() instead.
  float *TransformFloatVectorAtPoint(const float point[3],
                                     const float vector[3]) {
    this->TransformVectorAtPoint(point,vector,this->InternalFloatPoint);
    return this->InternalFloatPoint; };

  // Description:
  // Apply the transformation to a series of points, and append the
  // results to outPts.  
  virtual void TransformPoints(vtkPoints *inPts, vtkPoints *outPts);

  // Description:
  // Apply the transformation to a combination of points, normals
  // and vectors.  
  virtual void TransformPointsNormalsVectors(vtkPoints *inPts, 
                                             vtkPoints *outPts, 
                                             vtkDataArray *inNms, 
                                             vtkDataArray *outNms,
                                             vtkDataArray *inVrs, 
                                             vtkDataArray *outVrs);

  // Description:
  // Get the inverse of this transform.  If you modify this transform,
  // the returned inverse transform will automatically update.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              