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

#include "common/mathhelper.h"

#include <stdlib.h>
#include <math.h>

#include "math/linalg.h"
#include "common/constants.h"
#include "common/declspec.h"
#include "tools/odv.h"

/*! \file
  This file implements mathematical algorithms and helper functions.
*/

/**************************************************************************/
DECLSPEC
double adjustedValue(double v,double minVal,double maxVal,double offSet)
/**************************************************************************/
/*!
  \brief Returns a cyclicly adjusted value.

  \return
  - \a v if \a v is within the range [\a minVal, \a maxVal].
  - \a (v+offSet) if \a v is smaller than \a minVal.
  - \a (v-offSet) if \a v is larger than \a maxVal.
*/
{
  if      (v<minVal) return (v+offSet);
  else if (v>maxVal) return (v-offSet);
  else               return v;
}

/**************************************************************************/
DECLSPEC
void autoScale(double vLL,double vUR,double& adjLL,double& adjUR,
               double& ticStart,double& ticSpacing,double& lticSpacing,
               int axisType)
/**************************************************************************/
/*!
  \brief Returns adjusted, "nice" bounds \a adjLL, \a adjUR as well as
  tic start value \a ticStart, spacing of small tics \a ticSpacing and
  spacing of labeled tics \a lticSpacing given bounding values \a vLL
  and \a vUR.

  \a axisType specifies the type of axis and can have the following
  values: -2 color bar axis; -1 normal axis; 0 longitude axis; 1
  latitude axis.
*/
{
  const int mintic=3,maxti1=6,maxti2=12;
  int nltic;
  double range=fabs(vLL-vUR);

  /* special case vLL==ODV::missDOUBLE */
  if (vLL==ODV::missDOUBLE)
    { adjLL=ticStart=ODV::missDOUBLE; adjUR=ticSpacing=lticSpacing=ODV::largeDOUBLE; }

  /* special case of identical vLL and vUR */
  if (range==0.) { vLL-=1.; vUR+=1.; }

  lticSpacing=pow(10.,floor(log10(range)-0.5)); nltic=(int) (range/lticSpacing);

  if      (nltic<mintic)
    lticSpacing=lticSpacing*0.5;
  else if (nltic>maxti1 && nltic<=maxti2)
    lticSpacing=(axisType==-2) ? lticSpacing*2.5 : lticSpacing*2.;
  else if (nltic>maxti2)
    lticSpacing=lticSpacing*5.0;

  ticSpacing=lticSpacing*0.5;

  /* align adjLL and adjUR on ticSpacing boundaries; align ticStart on
     lticSpacing boundaries */
  if (vUR>vLL)
    {
      adjLL=floor(vLL/ticSpacing)*ticSpacing;
      adjUR=ceil(vUR/ticSpacing)*ticSpacing;
      ticStart=ceil(vLL/lticSpacing)*lticSpacing;
    }
  else if (vUR<vLL)
    {
      adjLL=ceil(vLL/ticSpacing)*ticSpacing;
      adjUR=floor(vUR/ticSpacing)*ticSpacing;
      ticStart=floor(vLL/lticSpacing)*lticSpacing;
      lticSpacing*=-1.; ticSpacing*=-1.;
    }
}

/**************************************************************************/
void autoLengthScales(int nData,double *X,double *Y,double *Z,
                      double vLeft,double vRight,double xLen,
                      double vBottom,double vTop,double yLen,
                      double &sxLengthScale,double &syLengthScale)
/**************************************************************************/
/*!

  \brief Determines appropriate X and Y gridding length scales \a
  sxLengthScale and \a syLengthScale from the set of \a nData data
  points \a X, \a Y, \a Z.

  \a X and \a Y are viewport coordinates of a window with viewport \a
  vLeft, \a vRight, \a vBottom, \a vTop. \a xLen and \a yLen are width
  and height of the window in cm.

*/
{
  int i,j,n,N=0,mxPts=qMin(100,nData); const double fac=2.25,rnd=0.05;

  /* choose randomly up to MAXPTS test points. the coordinates (tX,tY)
     of a test point are the perturbed coordinates of a data
     point. The magnitude of the perturbation is up to 5% of the axis
     range (rnd). for each of the N test point we will determine the
     closest data point and remember the minimal X- and Y-distances
     xDist, yDist to the test point. distances are scaled to [0,xlen]
     or [0,ylen]. */
  double dX=vRight-vLeft,dXi=xLen/dX,dY=vTop-vBottom,dYi=yLen/dY;
  double *tX=new double[mxPts],*tY=new double[mxPts];
  double *xDist=new double[mxPts],*yDist=new double[mxPts],tDist,d,dx,dy,dd;

  srand(1);
  while (N<mxPts)
    {
      /* choose a test point from the set of data points. ensure X and
         Y coordinates are valid. */
      i=(int)((double) rand()*(nData-1)/RAND_MAX);
      if (X[i]==ODV::missDOUBLE || Y[i]==ODV::missDOUBLE) continue;
      tX[N]=X[i]; tY[N]=Y[i];
      tDist=ODV::largeDOUBLE; xDist[N]=yDist[N]=ODV::missDOUBLE;

      /* perturb the test point */
      d=2*((double) rand()/RAND_MAX-0.5); // [-1. - 1.]
      tX[N]+=d*rnd*dX;
      d=2*((double) rand()/RAND_MAX-0.5); // [-1. - 1.]
      tY[N]+=d*rnd*dY;

      /* loop over all data points and find closest point to
         (tX,tY). require valid Z value. */
      for (j=0; j<nData; ++j)
        {
          dx=(X[j]-tX[N])*dXi; dy=(Y[j]-tY[N])*dYi; dd=dx*dx+dy*dy;
          if (Z[j]!=ODV::missDOUBLE && dx!=0. && dy!=0. && dd<tDist)
            { xDist[N]=fabs(dx); yDist[N]=fabs(dy); tDist=dd; }
        }

      ++N;
    }

  /* determine the average minimal X and Y distances dx and dy over
     all test points. set sxLen and syLen accordingly (units: percent
     of axis range). safeguards: range=[0.1, 25]; if ratio beyond
     rLim: use average. */
  dx=dy=0.; n=0;
  for (i=0; i<N; ++i)
    {
      if (xDist[i]!=ODV::missDOUBLE && yDist[i]!=ODV::missDOUBLE)
        { dx+=xDist[i]; dy+=yDist[i]; ++n; }
    }
  sxLengthScale=qBound(0.1,fac*100.*dx/xLen/n,25.);
  syLengthScale=qBound(0.1,fac*100.*dy/yLen/n,25.);
  double r=sxLengthScale/syLengthScale,rLim=5.;
  if (r<(1./rLim) || r>rLim)
    sxLengthScale=syLengthScale=0.5*(sxLengthScale+syLengthScale);

  // FILE *fp=fopen("c:\\atmp\\test.txt","w");
  // fprintf(fp,"Longitude\tLatitude\tDummy\tDist-X\tDist-Y\tX\tY\n");
  // for (i=0; i<N; ++i)
  //   {
  //     dx=tX[i]; dy=tY[i]; projector()->unproject(dx,dy);
  //     fprintf(fp,"%g\t%g\t0\t%g\t%g\t%g\t%g\n",dx,dy,xDist[i],yDist[i],tX[i],tY[i]);
  //   }
  // fclose(fp);

  /* free memory */
  delete[] xDist; delete[] yDist; delete[] tX; delete[] tY;
}

/**************************************************************************/
DECLSPEC
double boundedValue(double v,double validMin,double validMax,double dfltVal)
/**************************************************************************/
/*!
  \brief Returns a safeguarded value.

  \return
  - \a v if \a v is within the valid range [\a validMin, \a validMax].
  - \a dfltVal if \a v is outside the valid range.
*/
{
  if (v<validMin || v>validMax) return dfltVal;
  else                          return v;
}

/**************************************************************************/
DECLSPEC
int boundedValue(int v,int validMin,int validMax,int dfltVal)
/**************************************************************************/
/*!
  \brief Returns a safeguarded value.

  \return
  - \a v if \a v is within the valid range [\a validMin, \a validMax].
  - \a dfltVal if \a v is outside the valid range.
*/
{
  if (v<validMin || v>validMax) return dfltVal;
  else                          return v;
}

/**************************************************************************/
DECLSPEC
short boundedValue(short v,short validMin,short validMax,short dfltVal)
/**************************************************************************/
/*!
  \brief Returns a safeguarded value.

  \return
  - \a v if \a v is within the valid range [\a validMin, \a validMax].
  - \a dfltVal if \a v is outside the valid range.
*/
{
  if (v<validMin || v>validMax) return dfltVal;
  else                          return v;
}

/**************************************************************************/
DECLSPEC
void convertListValues(QList<double> *lst,QStringList *lblLst,
                       double a,double v0,double b,double c)
/**************************************************************************/
/*!
  \brief Converts values \a v in \a lst with: V = a * (v-v0) + b.

  \note Values V outside the range [b,c] are rejected.
*/
{
  QList<double> ol(*lst); int i,n=ol.size(); double v; QStringList lblOl;
  if (lblLst) { lblOl.append(*lblLst); lblLst->clear();}
  lst->clear();
  for (i=0; i<n; ++i)
  {
    v=a*(ol.at(i)-v0)+b;
    if (v>=b && v<=c)
      { lst->append(v); if (lblLst) lblLst->append(lblOl.at(i)); }
  }
}

/**************************************************************************/
DECLSPEC
double covariance(double x1,double x2,double y1,double y2,
                  double f,double sx,double sy)
/**************************************************************************/
/*!
 */
{
  double dx=(x1-x2)/sx,dy=(y1-y2)/sy,arg=dx*dx+dy*dy;
  return f*exp(-arg);
}

/**************************************************************************/
DECLSPEC
int defaultDecimalCount(double res)
/**************************************************************************/
/*!

  \brief Returns number of decimals required to resolve value
  differences of magnitude \a res.

*/
{
  int iDec=1+(int) -log10(fabs(res));
  return qMax(0,iDec);
}

/**************************************************************************/
DECLSPEC
int defaultDecimalCount(double r,double r3,double r2,double r1,double r0)
/**************************************************************************/
/*!
 */
{
  int idec=4;

  if      (r>=r3 && r<r2) idec=3;
  else if (r>=r2 && r<r1) idec=2;
  else if (r>=r1 && r<r0) idec=1;
  else if (r>=r0)         idec=0;

  return idec;
}

/**************************************************************************/
DECLSPEC
void doBinCounting(double *data,int nData,double left,double right,
                   int nBins,double *binMidCoords,double *binCounts,
                   int& firstBin,int& lastBin)
/**************************************************************************/
/*!

  \brief Performs bin-counting of \a nData data values \a data using
  \a nBins equidistant bins covering the interval [\a left, \a right].

  Mid-point bin coordinates and counts are returned at \a binMidCoords
  and \a binCounts, respectively. On exit \a firstBin and \a lastBin
  are the (0-based) first and last bin IDs containing data.

*/
{
  int i,ib; double dR=right-left,dr=dR/nBins,dRi=1./dR,b;

  /* define midpoints and initialize counts */
  for (i=0; i<nBins; ++i)
    { binMidCoords[i]=left+(i+0.5)*dr; binCounts[i]=0.; }

  /* loop over all data and count. data outside the [left, right]
     range are ignored. */
  for (i=0; i<nData; ++i)
    {
      b=(data[i]-left)*dRi;
      if (b>=0. && b<=1.)
        { ib=qMin((int) (b*nBins),nBins-1); ++binCounts[ib]; }
    }

  /* determine first and last bin IDs containing data */
  firstBin=lastBin=-1;
  for (i=0; i<nBins; ++i)
    if (binCounts[i]>0) { lastBin=i; if (firstBin==-1) firstBin=i; }
}

/**************************************************************************/
DECLSPEC
int findBin(double *B,int nB,double b)
/**************************************************************************/
/*!
  \brief Determines index \a i such that \a b is inside the interval
  [\a B[i], \aB[i+1]], given a monotonic set of nodes \a B[i], with
  i=0 ... \a nB-1.

  \return The determined index, or \c -1 if \a b is out of range.
*/
{
  int il,im,ir;

  if (B[1]>B[0]) { il=0; ir=nB-1; }
  else           { ir=0; il=nB-1; }
  if (b<B[il] || b>B[ir]) return -1;

  while (abs((ir-il))>1)
    {
      im=(il+ir)/2;
      if      (b>=B[il] && b<B[im]) ir=im;
      else if (b>=B[im] && b<=B[ir]) il=im;
    }
  return qMin(il,ir);
}

/**************************************************************************/
DECLSPEC
int findIndex(double *D,int n,double dVal,double tol)
/**************************************************************************/
/*!
  \brief Returns index \a i such that \a D[i] is "close" to \a dVal.

  To be considered "close", the relative distance bewteen \a D[i] and
  \a dVal must be less than \c qMax(fabs(tol*0.01),1.e-8). \a tol is a
  relative difference in percent.

  \c ODV::missINT32 is returned if no "close" value is found.
*/
{
  /* immediate return if no data */
  if (!D || !n) return ODV::missINT32;

  int i; double t=qMax(fabs(tol*0.01),1.e-12);

  for (i=0; i<n; ++i)
    if (!isDifferent(D[i],dVal,t)) return i;

  return ODV::missINT32;
}

/**************************************************************************/
DECLSPEC
int findLineIntersect(double x0,double y0,double x1,double y1,
                      double X0,double Y0,double X1,double Y1,double& x,double& y)
/**************************************************************************/
/*!
  \brief Calculates the coordinates \a x and \a y of the intersection
  of the two lines \a x0/\a y0 --> \a x1/\a y1 and \a X0/\a Y0 --> \a
  X1/\a Y1 (return value 1).

  If the lines are ill defined or coincide, 0 is returned.
*/
{
  int ier=0; double dx=x1-x0,dy=y1-y0,dX=X1-X0,dY=Y1-Y0,a,b,A,B;

  if (dx==0. && dy!=0. && dX!=0.)
    { A=dY/dX; B=Y0-A*X0; x=x0; y=A*x+B; ier=1; }
  else if (dX==0. && dY!=0. && dx!=0.)
    { a=dy/dx; b=y0-a*x0; x=X0; y=a*x+b; ier=1; }
  else if (dx!=0. && dX!=0.)
    {
      a=dy/dx; b=y0-a*x0; A=dY/dX; B=Y0-A*X0;
      if (a!=A) { x=(B-b)/(a-A); y=a*x+b; ier=1; }
    }

  return ier;
}

/**************************************************************************/
DECLSPEC
int findRectIntersect(double x0,double y0,double x1,double y1,
                      double xR0,double yR0,double xR1,double yR1,
                      double& x,double& y)
/**************************************************************************/
/*!
  If one of the points P0 \a x0/\a y0 and P1 \a x1/\a y1 is inside
  rectangle [\a xR0/\a yR0 - \a xR1/\a yR1] and the other point is
  outside, returns the coordinates \a x/\a y of the intersection of
  the connecting line between P0 and P1 with the rectangle frame
  (return value 1). if both points are inside or outside the
  rectangle, the return value is 0.
*/
{
  double xmin=qMin(x0,x1),xmax=qMax(x0,x1);
  double ymin=qMin(y0,y1),ymax=qMax(y0,y1);

  /* try all four frame segments */
  if (findLineIntersect(x0,y0,x1,y1,xR0,yR0,xR1,yR0,x,y) &&
      x>=xmin && x<=xmax && y>=ymin && y<=ymax) return 1;
  if (findLineIntersect(x0,y0,x1,y1,xR0,yR1,xR1,yR1,x,y) &&
      x>=xmin && x<=xmax && y>=ymin && y<=ymax) return 1;
  if (findLineIntersect(x0,y0,x1,y1,xR0,yR0,xR0,yR1,x,y) &&
      x>=xmin && x<=xmax && y>=ymin && y<=ymax) return 1;
  if (findLineIntersect(x0,y0,x1,y1,xR1,yR0,xR1,yR1,x,y) &&
      x>=xmin && x<=xmax && y>=ymin && y<=ymax) return 1;

  return 0;
}

/**************************************************************************/
DECLSPEC
void fitGeometryToBBGeometry(double& xlen,double& xof,double& ylen,double& yof,
                             double xlenBB,double xofBB,double ylenBB,double yofBB,
                             double lenRatio)
/**************************************************************************/
/*!
  \brief Calculates the geometry of a window with X/Y length ratio \a
  lenRatio centered in a given bounding geometry.

  On exit \a xlen, \a xof, \a ylen, \a yof contain the geometry of the
  centered window.
*/
{
  double dxmag=lenRatio,dymag=1.,xfak=qMin(xlenBB/dxmag,ylenBB/dymag);
  xlen=xfak*dxmag; ylen=xfak*dymag;
  xof=xofBB+(xlenBB-xlen)*0.5; yof=yofBB+(ylenBB-ylen)*0.5;
}

/**************************************************************************/
DECLSPEC
double FORTRAN_mod(double a1,double a2)
/**************************************************************************/
/*!
  \brief Mimicks FORTRAN mod function.
*/
{
  int n; double a,aa,r=0.;

  if (a1!=0. && a2!=0.)
    { a=a1/a2; aa=fabs(a); n=(int)aa; r=a1-a/aa*n*a2; }

  return r;
}

/**************************************************************************/
DECLSPEC
void indexx(int n,double arrin[],int indx[])
/**************************************************************************/
/*!
  \brief Indexes an array \a arrin of length \a n.

  Outputs the array \a indx such that \a arrin(indx(j)) is in
  ascending order for \a j=0,1,..\a n-1.  The input quantities \a n
  and \a arrin are not changed.

  from: Numerical Recipes
*/
{
  int l,j,ir,indxt,i; double q;

  for (j=1;j<=n;++j) indx[j-1]=j;
  if (n<2) { indx[0]=0; return; }
  l=(n >> 1) + 1; ir=n;

  for (;;)
    {
      if (l > 1)
        q=arrin[(indxt=indx[--l-1])-1];
      else
        {
          q=arrin[(indxt=indx[ir-1])-1];
          indx[ir-1]=indx[0];
          if (--ir == 1)
            {
              indx[0]=indxt;
              for (j=0;j<n;++j) indx[j]=indx[j]-1;
              return;
            }
        }

      i=l; j=l << 1;

      while (j <= ir)
        {
          if (j < ir && arrin[indx[j-1]-1] < arrin[indx[j]-1]) ++j;
          if (q < arrin[indx[j-1]-1])
            {
              indx[i-1]=indx[j-1];
              j += (i=j);
            }
          else j=ir+1;
        }
      indx[i-1]=indxt;
    }
}

/**************************************************************************/
DECLSPEC
void interpolate(int n,double *x,double *y,double missVal,
                 int nI,double *xI,double *yI)
/**************************************************************************/
/*!

  \brief Calculates and returns \a nI y values at \a xI[i] given \a n
  data pairs \a x[i] / \a y[i].

  \a missVal is returned for \a xI[i] outside the \a x value range, or
  if there are less than two data points. \a x and \a y values equal
  to \a missVal are ignored.

  \note the \a n \a x values must be monotonically increasing or
  decreasing.

  \note The memory at \a yI must be large enough to hold \a nI double
  values.

*/
{
  for (int i=0; i<nI; ++i)
    yI[i]=interpolatedValue(n,x,y,missVal,xI[i]);
}

/**************************************************************************/
DECLSPEC
double interpolatedValue(int n,double *x,double *y,double missVal,double xVal)
/**************************************************************************/
/*!

  \brief Calculates and returns the interpolated y value at \a xVal
  given \a n data pairs \a x[i] / \a y[i].

  \a missVal is returned if the \a x values do not have an interval
  containing \a xVal, or if there are less than two data points. \a x
  and \a y values equal to \a missVal are ignored.

  \note the \a n \a x values must be monotonically increasing or
  decreasing.

*/
{
  /* immediate return if less than two points or invalid data pointers */
  if (n<2 || !x || !y) return missVal;

  /* find first interval [x[i], x[i+1]] containing xVal and interpolate */
  double d=missVal,dx,dx0; int i,n0=n-1;
  for (i=0; i<n0; ++i)
    {
      dx=x[i+1]-x[i]; dx0=xVal-x[i];
      if (x[i]!=missVal && x[i+1]!=missVal && y[i]!=missVal && y[i+1]!=missVal &&
          fabs(dx)>0. && dx0*(xVal-x[i+1])<=0.)
        { d=y[i]+(y[i+1]-y[i])*dx0/dx; break; }
    }

  return d;
}

/**************************************************************************/
DECLSPEC
bool   isDifferent(double x1,double x2,double tol)
/**************************************************************************/
/*!
  \brief Checks whether the absolute value of the relative
  difference between \a x1 and \a x2 is larger than \a tol.

  \return \c true if difference is larger and \c false otherwise.
*/
{
  double denom=qMax(fabs(x1),fabs(x2)); denom=qMax(denom,XEPS);
  int i1=qIsFinite(x1),i2=qIsFinite(x2);
  return (!i1 || !i2 || (fabs(x1-x2)/denom)>tol);
}

/**************************************************************************/
DECLSPEC
bool   isDifferent(const QPointF& p1,const QPointF& p2,double tol)
/**************************************************************************/
/*!
  \brief Checks whether the x and y components of points \a p1 and \a p2
  differ by more than \a tol.

  \return \c true if difference is larger and \c false otherwise.
*/
{
  return (isDifferent(p1.x(),p2.x(),tol) || isDifferent(p1.y(),p2.y(),tol));
}

/**************************************************************************/
DECLSPEC
int iterateTowardsZero(int *iFlag,double Xleft,double Xright,double eps,
                       double *x,double *y,int maxit)
/**************************************************************************/
/*!
  \brief Drives \a x towards a zero-crossing of \a y in interval
  [\a Xleft,\a Xright].

  \a y may have the same sign at \a Xleft and \a Xright.

  \note \a iFlag must be zero initially.

  \return
  - 1 request new y at x
  - 2 success
  - 3 no change in sign found
  - 4 maximum no. of iterations reached
*/
{
  static int j1,j2,j3,it,m;
  static double a,b,h,u,x1,x2,y1,ya,yb,ytest;

  if ((*iFlag)>0) goto l30;
  a=Xleft; b=Xright; *x=a; j1=1; it=1; m=abs(maxit);

 l10: (*iFlag)=j1;

 l20: return (*iFlag);

 l30: if (fabs(*y)>eps) goto l50;

 l45: (*iFlag)=2; goto l20;

 l50: switch (j1)
    {
    case 1: goto l60;
    case 2: goto l70;
    case 3: goto l100;
    case 4: goto l170;
    }

 l60: ya=*y; *x=b; j1=2; goto l20;

 l70: if (ya*(*y)<0.) goto l120; x1=a; y1=ya; j1=3; h=b-a; j2=1;

 l80: x2=a+0.5*h; j3=1;

 l90: it=it+1; if (it>=m) goto l10; *x=x2; goto l20;

 l100: if (ya*(*y)<0.) goto l120; if (j3>=j2) goto l110;
  a=(*x); ya=(*y); x2=(*x)+h; j3=j3+1; goto l90;

 l110: a=x1; ya=y1; h=0.5*h; j2=j2+j2; goto l80;

 l120: b=(*x); yb=(*y); j1=4;

 l130: if (fabs(ya)<=fabs(yb)) goto l140;
  x1=a; y1=ya; *x=b; *y=yb; goto l150;

 l140: x1=b; y1=yb; *x=a; *y=ya;

 l150: u=(*y)*((*x)-x1)/((*y)-y1);

 l155: x2=(*x)-u; if (x2==(*x)) goto l195; x1=(*x); y1=(*y);
  ytest=.5*qMin(fabs(ya),fabs(yb)); if ((x2-a)*(x2-b)<0.0) goto l90;

 l160: x2=0.5*(a+b); ytest=0.; if ((x2-a)*(x2-b)<0.) goto l90; else goto l45;

 l170: if (ya*(*y)>=0.0) goto l180; b=*x; yb=*y; goto l190;

 l180: a=(*x); ya=(*y);

 l190: if (ytest<=0.0) goto l130;
  if ((fabs(*y)-ytest)<=0.) goto l150; else goto l160;

 l195: if (u==0.0) goto l45; u=u+u; goto l155;
}

/**************************************************************************/
DECLSPEC
int linest(int npo,double *x,double *y,double valmiss,
           int npos,double *sx,double *sy,double *sq,
           double& xMin,double& xMax,int& npFirst,int& npLast)
/**************************************************************************/
/*!
  \brief Interpolation of values \a sy at positions \a sx(i), i=0,\a
  npos-1 from original data \a x(i), \a y(i), \a i=0,\a npo-1 using
  piecewise linear least-squares.

  Values \a x(j) = \a valmiss or \a y(j) = \a valmiss indicate missing
  data and are not used for the interpolation. Estimated values beyond
  x-range of input data are linearly extrapolated using the first or
  last 2 observations for the line-fit.

  On exit input data in \a x, \a y and \a npo are unchanged; Errors \a
  sq are calculated assuming uncorrelated unit errors of the data. \a
  npFirst is the smallest (0-based) index for which \a sx[i] >= \a
  min(x[]), and \a npLast is the largest index for which \a sx[i] <=
  \a max(x[]). On exit, \a xMin and \a xMax contain the smallest and
  largest x values with valid y data.

  \note \a sx[] must be strictly monotonically increasing; data points
  in \a x and \a y can be in any order.

  \return 0 if successful and 1 otherwise.

  \sa linreg(), movingAverage()
*/
{
  double xp,xmin,xmax,xrange,xint,xlb=0.,xub=1.,xdist,fDX,dxMean,rangeLim=0.3;
  double xm,ym,slope,b0,var_slope,var_b0,cov_slb0,r,rms,res;
  int nval,id,idd,isd,i,i1,i2,ii1,ii2,n; bool bL,bR;
  int *ipwrk=0; double *X=0,*Y=0,*E=0;

  /* initialize values */
  npFirst=-1; npLast=-2;
  initArray<double>(sy,npos,valmiss);
  initArray<double>(sq,npos,-valmiss);

  /* error return if too few points */
  if (npo<=1) return 1;

  /* allocate space for work arrays */
  ipwrk=new int[npo]; X=new double[npo]; Y=new double[npo]; E=new double[npo];

  /* sort points in order of increasing x-values and copy to local array,
     do not use entries with missing data. determine xmin, xmax */
  xmin=ODV::largeDOUBLE; xmax=ODV::missDOUBLE; nval=-1; indexx(npo,x,ipwrk);
  for (id=0; id<npo; ++id)
    {
      i=ipwrk[id];
      if (x[i]!=valmiss && y[i]!=valmiss)
        {
          xmin=qMin(xmin,x[i]); xmax=qMax(xmax,x[i]); ++nval;
          X[nval]=x[i]; Y[nval]=y[i]; E[nval]=1.;
        }
    }

  /* look out for identical X values and average */
  id=1;
  while (id<=nval)
    {
      if (X[id]==X[id-1])
        {
          Y[id-1]=(Y[id-1]+Y[id])*0.5;
          for (idd=id; idd<nval; ++idd) { X[idd]=X[idd+1]; Y[idd]=Y[idd+1]; }
          --nval; --id;
        }
      ++id;
    }

  /* return if less than 2 points left or all points share the same x position */
  if (nval<1 || xmax==xmin)
    { delete[] ipwrk; delete[] X; delete[] Y; delete[] E; return 1; }

  /* set smallest and largest x values with valid y data */
  xMin=xmin; xMax=xmax;

  /* establish a default interval measure and mean interval width */
  fDX=50./(xmax-xmin); dxMean=(xmax-xmin)/npos;

  /* calculate the 'sy' values. 'nval+1' is total number of valid raw
     data stored in 'X' and 'Y' (i=0,...,nval) */
  for (isd=0; isd<npos; ++isd)
    {
      xp=sx[isd]; i1=-1; i2=-1; xrange=0.;
      if (npFirst==-1 && xp>=xmin) npFirst=isd;
      if (xp<=xmax) npLast=isd;

      /* determine active x-interval [xlb,xub] (width='xint')
         surrounding current point sx(isd). */
      if (npos>1)
        {
          /* determine width xint of active interval */
          if      (isd==0)        xint=fabs(sx[1]-sx[0]);
          else if (isd==(npos-1)) xint=fabs(xp-sx[isd-1]);
          else                    xint=fabs(sx[isd+1]-sx[isd-1])*0.5;

          /* set active interval [xlb, xub] */
          if      (xp<xmin) { xlb=xmin;         xub=xmin+2.*xint; }
          else if (xp>xmax) { xlb=xmax-2.*xint; xub=xmax; }
          else              { xlb=xp-xint;      xub=xp+xint; }
          fDX=1./(xub-xlb);

          /* determine smallest and largest points i1 and i2
             inside active x-interval. */
          for (i=0; i<=nval; ++i)
            { if (X[i]>=xlb && X[i]<=xub) { i1=i; break; } }
          for (i=nval; i>=0; --i)
            { if (X[i]>=xlb && X[i]<=xub) { i2=i; break; } }
        }

      /* if no or only one point is inside active x-interval, find the
         two closest points instead */
      if (i2==i1)
        {
          ii1=-1; ii2=-1;
          for (i=nval; i>=0; --i) { if (X[i]<=xp) { ii1=i; break; } }
          for (i=0; i<=nval; ++i) { if (X[i]> xp) { ii2=i; break; } }
          if (ii1==-1) ii1=ii2+1;
          if (ii2==-1) ii2=ii1-1;
          i1=qMin(ii1,ii2); i2=qMax(ii1,ii2);
        }

      /* 'xrange' measures range of active points in units of the active
         interval. if range is too small, also include next points. */
      if (i2>i1) xrange=fabs((X[i2]-X[i1])*fDX);
      bL=(X[i1]>xp && i1>0); bR=(X[i2]<xp && i2<nval);
      if (xrange<rangeLim && !bL && !bR) { bL=(i1>0); bR=(i2<nval); }
      while (bL || bR)
        {
          /* widen range of data values used */
          if (bL) { --i1; i1=qMax(0,i1); }
          if (bR) { ++i2; i2=qMin(nval,i2); }
          /* should we widen more */
          if (i2>i1) xrange=fabs((X[i2]-X[i1])*fDX);
          bL=(X[i1]>xp && i1>0); bR=(X[i2]<xp && i2<nval);
          if (xrange<rangeLim && !bL && !bR) { bL=(i1>0); bR=(i2<nval); }
        }

      /* calculate least-squares line using data between index i1 and i2 */
      if (xrange>=rangeLim)
        {
          linreg(i2-i1+1,X+i1,Y+i1,E+i1,
                 xm,ym,slope,b0,var_slope,var_b0,cov_slb0,r,rms,res);

          /* evaluate line at x */
          sy[isd]=b0+xp*slope;

          /* calculate error at xp */
          /*  	  sq[isd]=(float) sqrt(fabs(var_slope*xp*xp */
          /*  	  		+2.*cov_slb0*xp+var_b0)); */

          /* determine closest observation and set qality indicator */
          /* 	  xdist=-valmiss; */
          /* 	  for (i=i1; i<=i2; ++i) */
          /* 	    { xdist=qMin(xdist,(float) fabs(X[i]-xp)); } */
          /* 	  sq[isd]=xdist/(xub-xlb); */

          /* determine average distance of observation from x,
             and set qality indicator */
          xdist=0.; n=0;
          for (i=i1; i<=i2; ++i) { xdist+=fabs(X[i]-xp); ++n; }
          xdist/=(double) n; sq[isd]=xdist/qMax(dxMean,xub-xlb);
        }
    }

  delete[] ipwrk; delete[] X; delete[] Y; delete[] E;

  return 0;
}

/**************************************************************************/
DECLSPEC
double lineValueAt(double xVal,double x1,double y1,double x2,double y2)
/**************************************************************************/
/*!

  \brief Calculates and returns the interpolated y value at \a xVal
  given the two data pairs \a x1 / \a y1 and \a x2 / \a y2.

  \c ODV::missDOUBLE is returned if \a x1 and  \a x2 are equal.
*/
{
  double dx=x2-x1;
  if (dx==0.) return ODV::missDOUBLE;
  else        return y1+(y2-y1)*(xVal-x1)/dx;
}

/**************************************************************************/
DECLSPEC
int linreg(int npo,double *x,double *y,double *sdv_y,
           double& xmean,double& ymean,double& slope,double& b0,
           double& var_slope,double& var_b0,double& cov_slb0,
           double& r,double& rms,double& res)
/**************************************************************************/
/*!
  \brief Linear regression \a y = \a slope * \a x + \a b0 of data (\a
  x(i),\a y(i)) +- \a sdv_y(i),\a i=0,\a npo.

  Checks \a x and \a y values and excludes points that have \a x or \a
  y or both equal to ODV::missDOUBLE.

  \return \a slope and \a b0, their respective variances and
  covariances, the correlation coefficient between \a x and \a y, \a
  r, the root-mean-square deviation \a rms, and the reduced chi-square
  value \a res.

  \return The number of points used for the fit. Values less than 2
  indicate an error.

  \sa linest(), movingAverage()
*/
{
  int i,n; double a[15],denom,X,Y,varYi,dyy,dy;

  /* initialize variables */
  slope=b0=r=rms=res=var_slope=var_b0=cov_slb0=0.;

  /* must have at least two points */
  if (npo<2) return 0;

  /* accumulate information */
  for (i=0; i<15; ++i) a[i]=0.;
  for (n=0,i=0; i<npo; ++i)
    {
      X=x[i]; Y=y[i];
      if (X!=ODV::missDOUBLE && Y!=ODV::missDOUBLE)
        {
          varYi=1./(sdv_y[i]*sdv_y[i]);
          a[1]+=X*Y*varYi; a[2]+=X*varYi; a[3]+=X*X*varYi;
          a[4]+=Y*varYi; a[5]+=varYi; ++n;
        }
    }

  /* calculate mean and best line*/
  xmean=a[2]/a[5]; ymean=a[4]/a[5]; denom=a[3]*a[5]-a[2]*a[2];
  if (fabs(denom)<1.e-12) return 0;
  b0=(a[3]*a[4]-a[2]*a[1])/denom; slope=(a[1]-a[2]*b0)/a[3];

  /* calculate error information */
  var_slope=a[5]/denom; var_b0=a[3]/denom; cov_slb0=a[2]/denom;
  //r=cov_slb0/sqrt(var_slope*var_b0); //coorelation coeff between a and b

  /* calculate rms deviation and reduced chi-square */
  double N=n,dx,vX=0.,vY=0.,vXY=0.;
  for (i=0; i<npo; ++i)
    {
      X=x[i]; Y=y[i];
      if (X!=ODV::missDOUBLE && Y!=ODV::missDOUBLE)
        {
          dy=Y-(slope*X+b0); dyy=dy/sdv_y[i]; rms+=(dy*dy); res+=(dyy*dyy);
          dx=X-xmean; dy=Y-ymean; vX+=dx*dx; vY+=dy*dy; vXY+=dx*dy;
        }
    }
  rms=sqrt(rms/N); if (n>2) res=res/(N-2.);
  vX/=(N-1.); vY/=(N-1.); vXY/=(N-1.); r=vXY/sqrt(vX*vY);

  return n;
}

/**************************************************************************/
DECLSPEC
int linreg(int npo,double *x,double *y,
           double& xmean,double& ymean,double& slope,double& b0,
           double& var_slope,double& var_b0,double& cov_slb0,
           double& r,double& rms,double& res)
/**************************************************************************/
/*!
  \brief Linear regression \a y = \a slope * \a x + \a b0 of data (\a
  x(i),\a y(i)),\a i=0,\a npo.

  Assumes unit errors for all data points.

  Checks \a x and \a y values and excludes points that have \a x or \a
  y or both equal to ODV::missDOUBLE.

  \return \a slope and \a b0, their respective variances and
  covariances, the correlation coefficient \a r, the root-mean-square
  deviation \a rms, and the reduced chi-square value \a res.

  \return The number of points used for the fit. Values less than 2
  indicate an error.

  \sa linest(), movingAverage()
*/
{
  double *sdv_y=new double[npo]; initArray<double>(sdv_y,npo,1.);
  int n=linreg(npo,x,y,sdv_y,xmean,ymean,slope,b0,
               var_slope,var_b0,cov_slb0,r,rms,res);
  delete[] sdv_y;
  return n;
}

/**************************************************************************/
DECLSPEC
int mimaex(int n,int d[],int dmiss,int& dmin,int& dmax)
/**************************************************************************/
/*!
  \brief Determines minimal (\a dmin) and maximal (\a dmax) values of
  int values (\a d(i), \a i=0,\a n-1).

  Data-values equal to \a dmiss are ignored. If no valid point is
  found \a dmin is \c ODV::missINT32 and \a dmax is \c
  -#ODV::missINT32 on exit.

  \return Number of non-\a dmiss values in \a d.
*/
{
  int i,N;

  dmin=-ODV::missINT32; dmax=ODV::missINT32;
  if (n<=0 || !d) return 0;

  for (i=0,N=0; i<n; ++i)
    if (d[i]!=dmiss)
      {
        ++N;
        dmin=qMin(dmin,d[i]);
        dmax=qMax(dmax,d[i]);
      }

  if (!N)
    { dmin=ODV::missINT32; dmax=-ODV::missINT32; }

  return N;
}

/**************************************************************************/
DECLSPEC
int mimaex(int n,float d[],float dmiss,float& dmin,float& dmax)
/**************************************************************************/
/*!
  \brief Determines minimal (\a dmin) and maximal (\a dmax) values of
  float values (\a d(i), \a i=0,\a n-1).

  Data-values equal to \a dmiss are ignored. If no valid point is
  found \a dmin is \c ODV::missFLOAT and \a dmax is \c ODV::largeFLOAT
  on exit.

  \return Number of non-\a dmiss values in \a d.
*/
{
  int i,N;

  dmin=ODV::largeFLOAT; dmax=ODV::missFLOAT;
  if (n<=0 || !d) return 0;

  for (i=0,N=0; i<n; ++i)
    if (d[i]!=dmiss)
      {
        ++N;
        dmin=qMin(dmin,d[i]);
        dmax=qMax(dmax,d[i]);
      }

  if (!N)
    { dmin=ODV::missFLOAT; dmax=ODV::largeFLOAT; }

  return N;
}

/**************************************************************************/
DECLSPEC
int mimaex(int n,double d[],double dmiss,double& dmin,double& dmax)
/**************************************************************************/
/*!
  \brief Determines minimal (\a dmin) and maximal (\a dmax) values of
  double values (\a d(i), \a i=0,\a n-1).

  Data-values equal to \a dmiss are ignored. If no valid point is
  found \a dmin is \c ODV::missDOUBLE and \a dmax is \c
  ODV::largeDOUBLE on exit.

  \return Number of non-\a dmiss values in \a d.
*/
{
  int i,N; dmin=ODV::largeDOUBLE; dmax=ODV::missDOUBLE;
  if (n<=0 || !d) return 0;

  for (i=0,N=0; i<n; ++i)
    { if (d[i]!=dmiss) { ++N; dmin=qMin(dmin,d[i]); dmax=qMax(dmax,d[i]); } }

  if (!N) { dmin=ODV::missDOUBLE; dmax=ODV::largeDOUBLE; }

  return N;
}

/**************************************************************************/
DECLSPEC
int movingAverage(int npo,double *x,double *y,double valmiss,
                  int npos,double *sx,double *sy,double *scLen)
/**************************************************************************/
/*!
  \brief Estimation of values \a sy at positions \a sx(i), i=0,\a
  npos-1 from original data \a x(i), \a y(i), \a i=0,\a npo-1 using
  moving averages.

  \a scLen[j] is averaging scale-length [% axis range] at estimation
  point \a j.

  \note Estimates of \a valmiss indicate that there was no data point
  nearby.

  \return Number of points at which estimate was calculated.

  \sa linreg(), linest()
*/
{
  int i,j,n=0,ii; double S,W,w,X,dx,sxFac,d10,f;

  /* allocate memory for ExpValue and pre-calculate */
  double ExpValue[MAXEXPVALUES];
  for (i=0; i<MAXEXPVALUES; ++i) ExpValue[i]=exp(-i*EXPARGINC);

  /* loop over all estimation points */
  for (i=0; i<npos; ++i)
    {
      S=W=0; sy[i]=valmiss; X=sx[i]; sxFac=3./scLen[i];
      /* loop over all data points */
      for (j=0; j<npo; ++j)
        {
          if (x[j]!=valmiss && y[j]!=valmiss)
            {
              dx=fabs(X-x[j])*sxFac;
              if (dx<EXPMAXARG)
                {
                  /*  ww=exp(-dd); */
                  d10=EXPARGIINC*dx; ii=(int) d10; f=d10-ii;
                  w=(1.-f)*ExpValue[ii]+f*ExpValue[ii+1];
                  /* accumulate weighted parameters */
                  S+=y[j]*w; W+=w;
                }
            }
        }
      if (W>0.) { sy[i]=S/W; ++n; }
    }

  return n;
}

/**************************************************************************/
DECLSPEC
int myMinMax(int n,const double *d,double dMiss,double& dSpacing,
             int& minIdx,double& dMin,int& maxIdx,double& dMax)
/**************************************************************************/
/*!
  \brief Determines minimal (\a dMin) and maximal (\a dMax) values of
  double values (\a d(i), \a i=0,\a n-1).

  Also determines the (0-based) index values \a minIdx and \a maxIdx
  at which the minimal and maximal values occur, as well as the
  average increment between successive points \a dSpacing.

  Data-values equal to \a dMiss are ignored. If no valid point is
  found \a dMin is \c ODV::missDOUBLE and \a dMax is \c ODV::largeDOUBLE on exit.

  \return Number of non-\a dmiss values in \a d.
*/
{
  minIdx=maxIdx=-1; dMin=ODV::largeDOUBLE; dMax=ODV::missDOUBLE; dSpacing=0;
  if (!d) { dMin=ODV::missDOUBLE; dMax=ODV::largeDOUBLE; return 0; }

  /* loop over all points */
  int i,N,nSp=0; double dLast=ODV::missDOUBLE;
  for (i=0,N=0; i<n; ++i)
    {
      if (d[i]!=dMiss)
        {
          if (d[i]<dMin) { minIdx=i; dMin=d[i]; }
          if (d[i]>dMax) { maxIdx=i; dMax=d[i]; }
          if (dLast!=ODV::missDOUBLE) { dSpacing+=fabs(d[i]-dLast); ++nSp; }
          dLast=d[i]; ++N;
        }
    }

  /* calculate average spacing */
  if (nSp) dSpacing/=nSp;

  /* set proper dMin, dMax values if no valid point is found */
  if (!N) { dMin=ODV::missDOUBLE; dMax=ODV::largeDOUBLE; }

  return N;
}

/**************************************************************************/
DECLSPEC
double myround(double val,int decim)
/**************************************************************************/
/*!
  \brief Rounds double value \a val to \a decim decimal places.

  If fabs(\a val)/10<sup>\a decim</sup> < 1, \a val is left unchanged.
*/
{
  qint64 ival,sign; double fac,faci,sval,dval,dif;

  fac=pow(10.,(double) decim); faci=1./fac; sval=val*fac;
  /*   dval=fabs(val)*faci; */
  dval=fabs(val)*fac;

  if (fabs(sval)<1.e14 && sval!=0.)
    {
      if (dval>1.)
        {
          ival=(qint64) sval; dif=fabs(sval-((double) ival));
          sign=(qint64) (sval/fabs(sval));
          if (dif>=0.5) ival+=sign;
          val=((double) ival)*faci;
        }
      else val=myround(val,decim+1);
    }
  return val;
}

/**************************************************************************/
DECLSPEC
float myroundf(float val,int decim)
/**************************************************************************/
/*!
  \brief Rounds float value \a val to \a decim decimal places.

  If fabs(\a val)/10<sup>\a decim</sup> < 1, \a val is left unchanged.
*/
{
  return (float) myround((double) val,decim);
}

/**************************************************************************/
DECLSPEC
int nearestPoint(float x,float y,float *xp,float *yp,int np,
                 float xScale,float yScale)
/**************************************************************************/
/*!
  \brief Searches the \a np points \a xp/yp and determines the one that
  is nearest to \a x/y using normalized distances \a (xp-x)/xScale and
  \a (yp-y)/yScale.

  \return 0-based index of nearest point or -1 in case of errors.
*/
{
  int i,n=-1; float xF=xScale,yF=yScale,d,dx,dy,dMin=(float)1.e32;

  if (xScale!=0. && yScale!=0. && np>0)
    {
      xF=1./xF; yF=1./yF;
      for (i=0; i<np; ++i)
        {
          dx=(x-xp[i])*xF; dy=(y-yp[i])*yF; d=dx*dx+dy*dy;
          if (d<dMin) { n=i; dMin=d; }
        }
    }

  return n;
}

/**************************************************************************/
DECLSPEC
int nearestPoint(double x,double y,double *xp,double *yp,int np,
                 double xScale,double yScale)
/**************************************************************************/
/*!
  \brief For a given point \a x/y searches the \a np points \a xp/yp
  and determines the one that is nearest to \a x/y using normalized
  distances \a (xp-x)/xScale and \a (yp-y)/yScale.

  \return 0-based index of nearest point or -1 in case of errors.
*/
{
  int i,n=-1; double xF=xScale,yF=yScale,d,dx,dy,dMin=1.e32;

  if (xScale!=0. && yScale!=0. && np>0)
    {
      xF=1./xF; yF=1./yF;
      for (i=0; i<np; ++i)
        {
          dx=(x-xp[i])*xF; dy=(y-yp[i])*yF; d=dx*dx+dy*dy;
          if (d<dMin) { n=i; dMin=d; }
        }
    }

  return n;
}

/**************************************************************************/
DECLSPEC
int planeFit(int n,double *x,double *y,double *z,double& ax,double& ay,double& z0)
/**************************************************************************/
/*!
  \brief Calculates the coefficients of least-squares linear plane \a
  z = \a ax*x + \a ay*y + \a z0 through \a n data points (\a x, \a y,
  \a z).

  return values: 1 success, 0 singular problem or memory exhausted.
*/
{
  int i,j,k,ier=0;
  double a[9],p[3],d[3],b[3],a2[4],d2[2],dx,dy,N=(double) n,eps=1.e-8;

  /* initialize */
  for (i=0; i<3; ++i) { d[i]=0.; for (j=0; j<3; ++j) a[i*3+j]=0.; }

  /* loop over data points and build matrix a */
  for (k=0; k<n; ++k)
    {
      d[0]+=z[k]*x[k]; d[1]+=z[k]*y[k]; d[2]+=z[k];
      a[0]+=x[k]*x[k]; a[1]+=x[k]*y[k]; a[2]+=x[k];
      a[4]+=y[k]*y[k]; a[5]+=y[k]; a[8]+=1.;
    }
  dx=(a[0]-a[2]*a[2]/N)/(N-1.); dy=(a[4]-a[5]*a[5]/N)/(N-1.);

  /* default is a constant */
  ax=0.; ay=0.; z0=d[2]/N;

  /* do least-squares fit; deal with cases where x or y values don't vary */
  if (dx<eps)
    {
      for (i=0; i<2; ++i)
        { d2[i]=d[i+1]; for (j=0; j<2; ++j) a2[i*2+j]=a[(i+1)*3+j+1]; }

      if (LACholeskyDcmp(2,a2,p))
        {
          LAInvertCholDcmp(2,a2,p); LAMVMultiply(2,2,a2,d2,b);
          ax=0.; ay=b[0]; z0=b[1]; ier=1;
        }
    }
  else if (dy<eps)
    {
      d2[0]=d[0]; d2[1]=d[2]; a2[0]=a[0]; a2[1]=a[2]; a2[3]=a[8];

      if (LACholeskyDcmp(2,a2,p))
        {
          LAInvertCholDcmp(2,a2,p); LAMVMultiply(2,2,a2,d2,b);
          ay=0.; ax=b[0]; z0=b[1]; ier=1;
        }
    }
  else
    {
      if (LACholeskyDcmp(3,a,p))
        {
          LAInvertCholDcmp(3,a,p); LAMVMultiply(3,3,a,d,b);
          ax=b[0]; ay=b[1]; z0=b[2]; ier=1;
        }
    }
  return ier;
}

/**************************************************************************/
DECLSPEC
int polyFit(int ndegree,double *pC,int n,double *pX,double *pD,double *pSD)
/**************************************************************************/
/*!
  \brief Calculates the coefficients (returned at \a pC) of a
  polynomial of degree \a ndegree that fits the \a n data-points (\a
  pX,\a pD+-\a pSD) in a least-squares sense.

  f(x)=C0+C1*x+C2*x^2+ .... +Cm-1*x^(m-1)

  \return 1 if successful, 0 singular problem or memory exhausted.

  \note \c polyFit checks the \a pX and \a pD values and discards
  points with one or both coordinates equal to \a ODV::missDOUBLE. Standard
  deviations of 1 are assumed for all data points, if \a pSD is 0 on
  entry.
*/
{
  int i,j,k,result=0,m=ndegree+1;
  double *pA,*pP,*pRHS,sF,xFi,xFj,x,y;

  /* allocate memory for matrix and workspace */
  pP=new double[m]; pRHS=new double[m]; pA=new double[m*m];

  /* build RHS and A'*A matrix, loop over all data points */
  memset(pA,0,m*m*sizeof(double)); memset(pRHS,0,m*sizeof(double));
  for (k=0; k<n; ++k)
    {
      x=pX[k]; y=pD[k];
      if (x!=ODV::missDOUBLE && y!=ODV::missDOUBLE)
        {
          sF=pSD ? 1./(pSD[k]*pSD[k]):1.; xFi=1.;
          for (i=0; i<m; ++i)
            {
              pRHS[i]+=y*xFi*sF; xFj=1.;
              for (j=0; j<=i; ++j) { pA[j*m+i]+=xFi*xFj*sF; xFj*=x; }
              xFi*=x;
            }
        }
    }

  /* calculate Cholesky decomposition and inverse of A'*A */
  if ((result=LACholeskyDcmp(m,pA,pP)))
    { LAInvertCholDcmp(m,pA,pP); LAMVMultiply(m,m,pA,pRHS,pC); }

  /* delete all allocated memory */
  delete[] pP; delete[] pRHS; delete[] pA;

  return result;
}

/**************************************************************************/
DECLSPEC
double polyVal(int ndegree,double *pC,double x)
/**************************************************************************/
/*!
  \brief Evaluates polynomial of degree \a ndegree (coefficients at \a
  pC) at \a x.

  f(x)=C0+C1*x+C2*x^2+ .... +Cm-1*x^(m-1)
*/
{
  double y=0.;
  for (int i=ndegree; i>0; --i) y=x*(y+pC[i]);
  return y+pC[0];
}

/**************************************************************************/
DECLSPEC
double polyZero(int ndegree,double *pC,double x0)
/**************************************************************************/
/*!
  \brief Finds and returns zero-crossing of polynomial of degree \a
  ndegree (coefficients at \a pC) using Newton's method with starting
  point \a x0.

  f(x)=C0+C1*x+C2*x^2+ .... +Cm-1*x^(m-1)
*/
{
  int i,it=0,n1=ndegree-1;
  double *pD,yD,x=x0,y=polyVal(ndegree,pC,x),eps=1.e-8;

  /* allocate memory and define  coefficients of first derivative */
  pD=new double[ndegree];
  for (i=0; i<ndegree; ++i) *(pD+i)=(double) (i+1) *(*(pC+i+1));

  /* do Newton iterations until stopping criteria are met */
  while (fabs(y)>eps)
    { ++it; yD=polyVal(n1,pD,x); x-=y/yD; y=polyVal(ndegree,pC,x); }

  delete[] pD; return x;
}

/**************************************************************************/
DECLSPEC
int precisionFromRange(double r0,double r1)
/**************************************************************************/
/*!

  \brief Returns the precision necessary to resolve values in the range
  between \a r0 and \a r1.

*/
{
  double mr=qMax(1.,0.5*fabs(r0+r1)),dr=fabs(r1-r0);

  return (dr<1.e-12) ? 15 : (int) (log10(mr/dr))+4;
}

/**************************************************************************/
DECLSPEC
void setup1DGrid(double *X,int nX,double xFirst,double xLast,double dxFac)
/**************************************************************************/
/*!

  \brief Calculates \a nX grid points in \a X to encompass the [\a
  xFirst,\a xLast] interval.

  The generated values extend the [\a xFirst,\a xLast] interval by two
  grid-points on either side. \a X[2] is equal to \a xFirst and \a
  X[nX-3] is equal to \a xLast. The grid width increases by factor \a
  dxFac from point to point.

*/
{
  int i; double d,f,dx=1.;

  /* do initial intervals using 0 as start point and 1 as starting width */
  X[0]=0.; for (i=1; i<nX; ++i) { X[i]=X[i-1]+dx; dx*=dxFac; }

  /* now scale values so that X[2] is equal to xFirst and X[nX-3] is
     equal to xLast */
  d=X[2]; f=qMax(xLast-xFirst,0.001)/(X[nX-3]-d);
  for (i=0; i<nX; ++i) X[i]=xFirst+f*(X[i]-d);
}


/**************************************************************************/
DECLSPEC
bool validateRange(double &r0,double &r1)
/**************************************************************************/
/*!
  \brief Ensures that range [\a r0,\a r1] has finite width and adjusts
  \a r0 and \a r1 if necessary.

  \return \c true if changes were made, and \c false otherwise.
*/
{
  if (!isDifferent(r0,r1))
    {
      double d=qMax(1.e-3,fabs(0.05*(r0+r1)));
      d=qMin(1.,d);
      r0-=d; r1+=d;
      return true;
    }
  else
    return false;
}
