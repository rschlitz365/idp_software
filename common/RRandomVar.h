#ifndef RRANDOMVAR_H
#define RRANDOMVAR_H

/****************************************************************************
**
** Copyright (C) 1993-2021 Reiner Schlitzer. All rights reserved.
**
** This file is part of Ocean Data View.
**
** Class definitions of:
**                       RRandomVar
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


/**************************************************************************/
class RRandomVar
/**************************************************************************/
{
public:
  RRandomVar(int count,double *values,double missingValue);

  double maxValue() { evaluate(); return maxVal; }
  double mean();
  double median(double *dWrk=NULL,int *iWrk=NULL);
  double minValue() { evaluate(); return minVal; }
  int    nonMissValueCount();
  void   properties(int &nonMissValCount,double &mean,double &stDev,
		    double &minimumVal,double &maximumVal);
  double standardDeviation();
  double variance();

private:
  void evaluate();
  
  double *vals; //!< Pointer to the values (realizations)
  int valCount; //!< Number of values in vals
  double missVal; //!< Missing value indicator

  int nonMissCount; //!< Number of non-miss values in vals
  double mu; //!< Mean value
  double var; //!< Variance
  double med; //!< Median
  double maxVal; //!< Maximal value
  double minVal; //!< Minimal value
};

#endif	/* !RRANDOMVAR_H */
