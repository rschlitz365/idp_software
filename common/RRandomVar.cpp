/****************************************************************************
**
** Copyright (C) 1993-2021 Reiner Schlitzer. All rights reserved.
**
** This file is part of Ocean Data View.
**
** Class implementations of:
**                           RRandomVar
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <math.h>

#include "RRandomVar.h"

#include "globalFunctions.h"

/***************************************************************************
** NEW CLASS
***************************************************************************/

/*!
  \class RRandomVar

  \brief Given a set of double values provides functions for
  calculating mean, median, variance and standard deviation.

  \note It is assumed that the pointer to the double values \a values
  remains valid during the entire lifetime of this object.

*/

/**************************************************************************/
RRandomVar::RRandomVar(int count,double *values,double missingValue)
/**************************************************************************/
/*!
  \brief Sets up a RRandomVar with \a count values in \a values.

  If \a missingValue is the missing value indicator in \a values.
*/
{
  vals=values; valCount=count; missVal=missingValue;
  mu=var=med=missVal; minVal=1.e99; maxVal=-1.e99;
  nonMissCount=-1;
}

/**************************************************************************/
void RRandomVar::evaluate()
/**************************************************************************/
/*!
  \brief Calculates mean value and variance.
*/
{
  if (nonMissCount>-1) return;

  double s=0.,s2=0.,sn,v; int i,n=0;
  minVal=1.e99; maxVal=-1.e99;
  for (i=0; i<valCount; ++i)
    {
      v=vals[i];
      if (v!=missVal)
	{
	  s+=v; s2+=v*v; n+=1;
	  if (v>maxVal) maxVal=v;
	  if (v<minVal) minVal=v;
	}
    }
  nonMissCount=n; sn=n;

  /* calculate mean and standard deviation */
  if (n>0) { mu=s/sn; if (n>1) var=qMax(0.,(s2-s*mu)/(sn-1.)); }
  else     { minVal=missVal; maxVal=missVal; }
}

/**************************************************************************/
double RRandomVar::mean()
/**************************************************************************/
/*!

  \brief \return The mean value, or \a missVal if the mean cannot be
  calculated, for instance because of too few values.
*/
{
  if (nonMissCount==-1) evaluate();
  return mu;
}

/**************************************************************************/
double RRandomVar::median(double *dWrk,int *iWrk)
/**************************************************************************/
/*!

  \brief Calculates and returns the median of the values at member
  variable \a vals.

  Only values different from member variable \a missVal are
  considered.  \a missVal is returned if no valid input value is
  found.

  \note If provided, the workspace memory at \a dWrk and \a iWrk must
  be large enough to hold \a nonMissCount \c double and \c int values,
  respectively.

*/
{
  if (med!=missVal || nonMissCount==0) return med;
  
  bool dWrkAllocated=(dWrk==NULL),iWrkAllocated=(iWrk==NULL);
  if (dWrkAllocated) dWrk=new double[valCount];
  if (iWrkAllocated) iWrk=new int[valCount];
  
  int i,n=0,nc; double d;
  for (i=0; i<valCount; ++i)
    if (vals[i]!=missVal) { dWrk[n]=vals[i]; iWrk[n]=n; ++n; }

  if (n)
    {
      indexx(n,dWrk,iWrk); nc=(n-1)/2; d=dWrk[iWrk[nc]];
      /* odd/even n distinction */
      med=(n&1) ? d : 0.5*(d+dWrk[iWrk[nc+1]]);
    }

  if (dWrkAllocated) delete[] dWrk;
  if (iWrkAllocated) delete[] iWrk;

  return med;
}

/**************************************************************************/
int RRandomVar::nonMissValueCount()
/**************************************************************************/
/*!
  \brief \return The number of non-miss values in \a vals.
*/
{
  if (nonMissCount==-1) evaluate();
  return nonMissCount;
}

/**************************************************************************/
void RRandomVar::properties(int &nonMissValCount,double &mean,double &stDev,
			    double &minimumVal,double &maximumVal)
/**************************************************************************/
/*!

  \brief \return The properties of the RRandomVar object.
*/
{
  if (nonMissCount==-1) evaluate();
  nonMissValCount=nonMissCount; mean=mu; stDev=standardDeviation();
  minimumVal=minVal; maximumVal=maxVal;
}

/**************************************************************************/
double RRandomVar::standardDeviation()
/**************************************************************************/
/*!

  \brief \return The standard deviation, or \a missVal if the standard
  deviation cannot be calculated, for instance because of too few
  values.
*/
{
  if (nonMissCount==-1) evaluate();
  return (var>=0.) ? sqrt(var) : missVal;
}

/**************************************************************************/
double RRandomVar::variance()
/**************************************************************************/
/*!

  \brief \return The variance value, or \a missVal if the variance
  cannot be calculated, for instance because of too few values.
*/
{
  if (nonMissCount==-1) evaluate();
  return var;
}
