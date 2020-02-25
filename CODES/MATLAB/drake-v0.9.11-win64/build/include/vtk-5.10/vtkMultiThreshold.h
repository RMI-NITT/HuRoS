/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkMultiThreshold.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*----------------------------------------------------------------------------
 Copyright (c) Sandia Corporation
 See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.
----------------------------------------------------------------------------*/

// .NAME vtkMultiThreshold - Threshold cells within multiple intervals
// .SECTION Description
// This filter can be substituted for a chain of several vtkThreshold filters
// and can also perform more sophisticated subsetting operations.
// It generates a vtkMultiBlockDataSet as its output.
// This multiblock dataset contains a vtkUnstructuredGrid for each thresholded
// subset you request.
// A thresholded subset can be a set defined by an interval over a
// point or cell attribute of the mesh; these subsets are called IntervalSets.
// A thresholded subset can also be a boolean combination of one or more IntervalSets;
// these subsets are called BooleanSets.
// BooleanSets allow complex logic since their output
// can depend on multiple intervals over multiple variables
// defined on the input mesh.
// This is useful because it eliminates the need for thresholding several
// times and then appending the results, as can be required with vtkThreshold
// when one wants to remove some range of values (e.g., a notch filter).
// Cells are not repeated when they belong to more than one interval unless
// those intervals have different output grids.
//
// Another advantage this filter provides over vtkThreshold is the ability
// to threshold on non-scalar (i.e., vector, tensor, etc.) attributes without
// first computing an array containing some norm of the desired attribute.
// vtkMultiThreshold provides \f$L_1\f$, \f$L_2\f$, and \f$L_{\infty}\f$ norms.
//
// This filter makes a distinction between intermediate subsets and
// subsets that will be output to a grid.
// Each intermediate subset you create with AddIntervalSet or
// AddBooleanSet is given a unique integer identifier (via the return
// values of these member functions).
// If you wish for a given set to be output, you must call
// OutputSet and pass it one of these identifiers.
// The return of OutputSet is the integer index of the output set
// in the multiblock dataset created by this filter.
//
// For example, if an input mesh defined three attributes T, P, and s, one might
// wish to find cells that satisfy "T < 320 [K] && ( P > 101 [kPa] || s < 0.1 [kJ/kg/K] )".
// To accomplish this with a vtkMultiThreshold filter,
// <pre>
// vtkMultiThreshold* thr;
// int intervalSets[3];
//
// intervalSets[0] = thr->AddIntervalSet( vtkMath::NegInf(), 320., vtkMultiThreshold::CLOSED, vtkMultiThreshold::OPEN,
//     vtkDataObject::FIELD_ASSOCIATION_POINTS, "T", 0, 1 );
// intervalSets[1] = thr->AddIntervalSet( 101., vtkMath::Inf(), vtkMultiThreshold::OPEN, vtkMultiThreshold::CLOSED,
//     vtkDataObject::FIELD_ASSOCIATION_CELLS, "P", 0, 1 );
// intervalSets[2] = thr->AddIntervalSet( vtkMath::NegInf(), 0.1, vtkMultiThreshold::CLOSED, vtkMultiThreshold::OPEN,
//     vtkDataObject::FIELD_ASSOCIATION_POINTS, "s", 0, 1 );
//
// int intermediate = thr->AddBooleanSet( vtkMultiThreshold::OR, 2, &intervalSets[1] );
//
// int intersection[2];
// intersection[0] = intervalSets[0];
// intersection[1] = intermediate;
// int outputSet = thr->AddBooleanSet( vtkMultiThreshold::AND, 2, intersection );
//
// int outputGridIndex = thr->OutputSet( outputSet );
// thr->Update();
// </pre>
// The result of this filter will be a multiblock dataset that contains a single child with the desired cells.
// If we had also called <code>thr->OutputSet( intervalSets[0] );</code>, there would be two child meshes and
// one would contain all cells with T < 320 [K].
// In that case, the output can be represented by this graph
// \dot
// digraph MultiThreshold {
//   set0 [shape=rect,style=filled,label="point T(0) in [-Inf,320["]
//   set1 [shape=rect,label="cell P(0) in ]101,Inf]"]
//   set2 [shape=rect,label="point s(0) in [-Inf,0.1["]
//   set3 [shape=rect,label="OR"]
//   set4 [shape=rect,style=filled,label="AND"]
//   set0 -> set4
//   set1 -> set3
//   set2 -> set3
//   set3 -> set4
// }
// \enddot
// The filled rectangles represent sets that are output.

#ifndef __vtkMultiThreshold_h
#define __vtkMultiThreshold_h

#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkMath.h" // for Inf() and NegInf()

#include <vector> // for lists of threshold rules
#include <map> // for IntervalRules map
#include <set> // for UpdateDependents()
#include <string> // for holding array names in NormKey

class vtkCell;
class vtkCellData;
class vtkDataArray;
class vtkGenericCell;
class vtkPointSet;
class vtkUnstructuredGrid;

class VTK_GRAPHICS_EXPORT vtkMultiThreshold : public vtkMultiBlockDataSetAlgorithm
{
public:
  vtkTypeMacro(vtkMultiThreshold,vtkMultiBlockDataSetAlgorithm);
  static vtkMultiThreshold* New();
  virtual void PrintSelf( ostream& os, vtkIndent indent );

  //BTX
  /// Whether the endpoint value of an interval should be included or excluded.
  enum Closure {
    OPEN=0,   //!< Specify an open interval
    CLOSED=1  //!< Specify a closed interval
  };
  /// Norms that can be used to threshold vector attributes.
  enum Norm {
    LINFINITY_NORM=-3, //!< Use the \f$L_{\infty}\f$ norm for the specified array threshold.
    L2_NORM=-2,        //!< Use the \f$L_2\f$ norm for the specified array threshold.
    L1_NORM=-1         //!< Use the \f$L_1\f$ norm for the specified array threshold.
  };
  /// Operations that can be performed on sets to generate another set. Most of these operators take 2 or more input sets.
  enum SetOperation {
    AND, //!< Only include an element if it belongs to all the input sets
    OR, //!< Include an element if it belongs to any input set
    XOR, //!< Include an element if it belongs to exactly one input set
    WOR, //!< Include elements that belong to an odd number of input sets (a kind of "winding XOR")
    NAND //!< Only include elements that don't belong to any input set
  };
  //ETX

  // Description:
  // Add a mesh subset to be computed by thresholding an attribute of the input mesh.
  // The subset can then be added to an output mesh with OuputSet() or combined with other sets using AddBooleanSet.
  // If you wish to include all cells with values below some number \a a, call
  // with xmin set to vtkMath::NegInf() and xmax set to \a a.
  // Similarly, if you wish to include all cells with values above some number \a a,
  // call with xmin set to \a a and xmax set to vtkMath::Inf().
  // When specifying Inf() or NegInf() for an endpoint, it does not matter whether
  // you specify and open or closed endpoint.
  // 
  // When creating intervals, any integers can be used for the IDs of output meshes.
  // All that matters is that the same ID be used if intervals should output to the same mesh.
  // The outputs are ordered with ascending IDs in output block 0.
  //
  // It is possible to specify an invalid interval, in which case these routines will return -1.
  // Invalid intervals occur when
  // - an array does not exist,
  // - \a center is invalid,
  // - \a xmin == \a xmax and \a omin and/or \a omax are vtkMultiThreshold::OPEN, or
  // - \a xmin > \a xmax.
  // - \a xmin or \a xmax is not a number (i.e., IEEE NaN). Having both \a xmin and \a xmax equal NaN is allowed.
  // vtkMath provides a portable way to specify IEEE infinities and Nan.
  // Note that specifying an inte                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 