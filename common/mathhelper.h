#ifndef MATHHELPER_H
#define MATHHELPER_H

/****************************************************************************
**
** Copyright (C) 1993-2025 Reiner Schlitzer. All rights reserved.
**
** This file is part of Ocean Data View.
**
** Global functions: mathematical helper functions
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*! \file
  This is the include file for mathematical algorithms and helper functions.
*/

#include <QList>
#include <QPointF>

#include "common/declspec.h"

DECLSPEC
double adjustedValue(double v,double minVal,double maxVal,double offSet);
DECLSPEC
void   autoScale(double vLL,double vUR,double& adjLL,double& adjUR,
    double& ticStart,double& ticSpacing,double& lticSpacing,int axisType=-1);
DECLSPEC
void   autoLengthScales(int nData,double *X,double *Y,double *Z,
      double vLeft,double vRight,double xLen,
      double vBottom,double vTop,double yLen,
      double &sxLengthScale,double &syLengthScale);
DECLSPEC
double boundedValue(double v,double validMin,double validMax,double dfltVal);
DECLSPEC
int    boundedValue(int v,int validMin,int validMax,int dfltVal);
DECLSPEC
short  boundedValue(short v,short validMin,short validMax,short dfltVal);
DECLSPEC
void   convertListValues(QList<double> *lst,QStringList *lblLst,
                        double a,double v0,double b,double c);
DECLSPEC
double covariance(double x1,double x2,double y1,double y2,
      double f,double sx,double sy);
DECLSPEC
int    defaultDecimalCount(double res);
DECLSPEC
int    defaultDecimalCount(double r,double r3,double r2,double r1,double r0);
DECLSPEC
void   doBinCounting(double *data,int nData,double left,double right,
        int nBins,double *binMidCoords,double *binCounts,
        int& firstBin,int& lastBin);
DECLSPEC
int    findBin(double *B,int nB,double b);
DECLSPEC
int    findIndex(double *D,int n,double dVal,double tol=0.);
DECLSPEC
int    findLineIntersect(double x0,double y0,double x1,double y1,
      double X0,double Y0,double X1,double Y1,
      double& x,double& y);
DECLSPEC
int    findRectIntersect(double x0,double y0,double x1,double y1,
      double xR0,double yR0,double xR1,double yR1,
      double& x,double& y);
DECLSPEC
void   fitGeometryToBBGeometry(double& xlen,double& xof,double& ylen,double& yof,
            double xlenBB,double xofBB,double ylenBB,double yofBB,
            double lenRatio);
DECLSPEC
double FORTRAN_mod(double a1,double a2);

/**************************************************************************/
template <typename T>
int indexOf(T value,const T *t,int count)
/**************************************************************************/
/*!

  \brief Determines the 0-based index of \a value in array \a t.

  \a count is the number of elements in \a t.

  \return The index of \a value in array \a t, or \c -1 if \a value is
  not found in \a t.

*/
{
  for (int i=0; i<count; ++i)
    if (t[i]==value) return i;

  return -1;
}

DECLSPEC
void   indexx(int n,double arrin[],int indx[]);

/**************************************************************************/
template <typename T>
void initArray(T *t,int n,T value)
/**************************************************************************/
/*!
  \brief Initializes \a n elements of \a t with value \a value,
  starting at first position.
*/
{
  for (int i=0; i<n; ++i) t[i]=value;
}

/**************************************************************************/
template <typename T>
void initArrayRange(T *t,int start,int end,T value)
/**************************************************************************/
/*!
  \brief Initializes elements of array \a t from position \a start to
  \a end with value \a value.
*/
{
  for (int i=start; i<=end; ++i) t[i]=value;
}

/**************************************************************************/
template<typename T>
bool isInRange(T p,T minInclusive,T maxInclusive)
/**************************************************************************/
/*!

  \return \c true if \a p is inside the interval [ \a minInclusive ,
  \a maxInclusive ], or \c false otherwise.

*/
{ return minInclusive<=p && p<=maxInclusive; }

DECLSPEC
void interpolate(int n,double *x,double *y,double missVal,
                 int nI,double *xI,double *yI);
DECLSPEC
double interpolatedValue(int n,double *x,double *y,double missVal,double xVal);
DECLSPEC
bool   isDifferent(double x1,double x2,double tol=1.e-9);
DECLSPEC
bool   isDifferent(const QPointF& p1,const QPointF& p2,double tol=1.e-9);
DECLSPEC
int    iterateTowardsZero(int *iFlag,double Xleft,double Xright,double eps,
                          double *x,double *y,int maxit);
DECLSPEC
int    linest(int npo,double *x,double *y,double valmiss,
              int npos,double *sx,double *sy,double *sq,
              double& xMin,double& xMax,int& npFirst,int& npLast);
DECLSPEC
double lineValueAt(double xVal,double x1,double y1,double x2,double y2);
DECLSPEC
int    linreg(int npo,double *x,double *y,double *sdv_y,
              double& xmean,double& ymean,double& slope,double& b0,
              double& var_slope,double& var_b0,double& cov_slb0,
              double& r,double& rms,double& res);
DECLSPEC
int    linreg(int npo,double *x,double *y,
              double& xmean,double& ymean,double& slope,double& b0,
              double& var_slope,double& var_b0,double& cov_slb0,
              double& r,double& rms,double& res);
DECLSPEC
int    mimaex(int n,int d[],int dmiss,int& dmin,int& dmax);
DECLSPEC
int    mimaex(int n,float d[],float dmiss,float& dmin,float& dmax);
DECLSPEC
int    mimaex(int n,double d[],double dmiss,double& dmin,double& dmax);
DECLSPEC
int    movingAverage(int npo,double *x,double *y,double valmiss,
                     int npos,double *sx,double *sy,double *scLen);
DECLSPEC
int    myMinMax(int n,const double *d,double dMiss,double& dSpacing,
                int& minIdx,double& dMin,int& maxIdx,double& dMax);
DECLSPEC
double myround(double val,int decim);
DECLSPEC
float  myroundf(float val,int decim);
DECLSPEC
int    nearestPoint(float x,float y,float *xp,float *yp,int np,
                    float xScale,float yScale);
DECLSPEC
int    nearestPoint(double x,double y,double *xp,double *yp,int np,
                    double xScale,double yScale);
DECLSPEC
int    planeFit(int n,double *x,double *y,double *z,
                double& ax,double& ay,double& z0);
DECLSPEC
int    polyFit(int ndegree,double *pC,int n,double *pX,double *pD,double *pSD=0);
DECLSPEC
double polyVal(int ndegree,double *pC,double x);
DECLSPEC
double polyZero(int ndegree,double *pC,double x0);
DECLSPEC
int    precisionFromRange(double r0,double r1);
DECLSPEC
void   setup1DGrid(double *X,int nX,double xFirst,double xLast,double dxFac);
DECLSPEC
bool   validateRange(double &r0,double &r1);

#endif	/* !MATHHELPER_H */
