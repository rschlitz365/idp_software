/****************************************************************************
**
** Copyright (C) 2025 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "UnitConverter.h"

#include <QFile>
#include <QTextStream>

//#include "common.h"


/**************************************************************************/
UnitConverter::UnitConverter(const QString& fn)
/**************************************************************************/
/*!

  \brief Appends the conversions from file \a fn to the conversion list.

*/
{
  /* append "As provided." conversion at index 0 */
  append("<any>","<any>","<any>","1.","0.","As provided.");
  
  /* append unit conversions from file fn */
  QFile f(fn); if (!f.exists() || !f.open(QIODevice::ReadOnly)) return;
  QTextStream in(&f); in.setCodec("UTF-8");
  int idx; QString l,s; QStringList sl; double fac,off; bool okF,okO;

  l=in.readLine();
  while (!in.atEnd())
    {
      l=in.readLine(); sl=l.split("\t");
      if (sl.size()==6)
	{
	  if ((idx=indexOf(sl.at(0),sl.at(1),sl.at(2)))>-1) continue;
	  fac=sl.at(3).toDouble(&okF); off=sl.at(4).toDouble(&okO);
	  if (okF && okO) append(sl.at(0),sl.at(1),sl.at(2),
				 sl.at(3),sl.at(4),sl.at(5));
	}
    }
}

/**************************************************************************/
int UnitConverter::append(const QString& varName,
			  const QString& fromUnits,const QString& toUnits,
			  const QString& factor,const QString& offset,
			  const QString& descriptiveText)
/**************************************************************************/
/*!

  \brief Appends the provided conversion to the conversion list.

  \return The size of this container after append.

*/
{
  var.append(varName); descr.append(descriptiveText);
  from.append(fromUnits); to.append(toUnits);
  fac.append(factor); off.append(offset);

  return size();
}

/**************************************************************************/
QString UnitConverter::description(int index)
/**************************************************************************/
/*!

  \brief Returns the descriptive text for conversion at 0-based index \a index.

*/
{
  QString s=descr.at(index);
  s.replace("$FROM_UNITS$",from.at(index));
  s.replace("$TO_UNITS$",to.at(index));

  return s;
}

/**************************************************************************/
int UnitConverter::indexOf(const QString& varName,
			   const QString& fromUnits,const QString& toUnits)
/**************************************************************************/
/*!

  \brief Returns the index in the conversion list of the conversion
  for variable \a varName from \a fromUnits to \a toUnits, or \c -1,
  if no such conversion exists.

*/
{
  int i,n=size();

  /* return 0 if from- and to- units are identical */
  if (fromUnits==toUnits) return 0;

  /* try to find the conversion in the list */
  for (i=0; i<n; ++i)
    if ((var.at(i)=="<any>" || var.at(i)==varName) &&
	from.at(i)==fromUnits && to.at(i)==toUnits) return i;

  /* conversion is unknown */
  return -1;
}

/**************************************************************************/
int UnitConverter::writeSortedListToFile(const QString& fn)
/**************************************************************************/
/*!

  \brief Writes a sorted list of conversions to file \a fn.

*/
{
  int i,n=size(); QMap<QString,QString> cnvs; QString key,entry;

  /* loop over all conversions and construct cnvs */
  for (i=0; i<n; ++i)
    {
      key=QString("%1_%2_%3").arg(var.at(i)).arg(from.at(i)).arg(to.at(i));
      entry=QString("%1\t%2\t%3\t%4\t%5\t%6")
	.arg(var.at(i)).arg(from.at(i)).arg(to.at(i))
	.arg(fac.at(i)).arg(off.at(i)).arg(descr.at(i));
      cnvs.insert(key,entry);
    }
  
  /* open the file for writing, create output stream and write header line */
  QFile fi(fn); if (!fi.open(QIODevice::Text | QIODevice::WriteOnly)) return 0;
  QTextStream out(&fi);
  out << QString("Variable\tFrom\tTo\tCnvFac\tCnvOff\tText") << endl;

  /* iterate over cnvs and write conversions to file fn */
  QString ele,lastEle; QMap<QString,QString>::const_iterator it=cnvs.constBegin();
  while (it!=cnvs.constEnd())
    {
      ele=it.key(); i=ele.indexOf('_'); if (i>-1) ele=ele.left(i);
      if (ele!=lastEle) { out << endl; lastEle=ele; }

      out << it.value() << endl; ++it;
    }
  
  return n;
}
