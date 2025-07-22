/****************************************************************************
**
** Copyright (C) 2019 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "Replacer.h"


/**************************************************************************/
void Replacer::append(RConfig& cf)
/**************************************************************************/
/*!

  \brief Appends pre/post strings from \c Substitutions group of \a cf.

*/
{
  cf.setGroup("Substitutions"); QStringList preLst=cf.groupEntryKeys();
  int n; QString s,from,to; QStringList sl;
  
  foreach (s, preLst)
    {
      sl=cf.getEntry(s).split(" >>> "); n=sl.size(); from=to=QString();
      if (n>0) from=sl.at(0).trimmed();
      if (n>1) to=sl.at(1).trimmed();

      if (!from.isEmpty()) { pre.append(from); post.append(to); }
    }
}

/**************************************************************************/
void Replacer::append(const QString& preString,const QString& postString)
/**************************************************************************/
/*!

  \brief Appends pre/post strings \a preString / \a postString.

*/
{
  pre.append(preString); post.append(postString);
}

/**************************************************************************/
void Replacer::appendStandardReplacements(const QString& cruise)
/**************************************************************************/
/*!

  \brief Appends standard meta- and data variable replacements.

*/
{
  append("CTDTEMP","CTDTMP");
  append("Orig_stn","Cast Identifier:INDEXED_TEXT");
  append("ODV_type","Type");
  append("Gear","Sampling Device:INDEXED_TEXT");
  append("Latitude[deg+veN]","Latitude[degrees_north]");
  append("Longitude[deg+veE]","Longitude[degrees_east]");
  append("W_depth[metres]","Bot. Depth[m]");
  append("W_depth[meters]","Bot. Depth[m]");
  append("yyyy-mm-ddThh:mi:ss","yyyy-mm-ddThh:mm:ss");
  append("yyyy-mm-ddThh24:mi:ss[GMT]","yyyy-mm-ddThh:mm:ss");
  append("yyyy-mm-ddThh24:mi:ss","yyyy-mm-ddThh:mm:ss");
  //append("[Dmnless]","");
  //append("Site","Station:METAVAR:INDEXED_TEXT");
  append("CRUISE","Cruise");
  append("Site","Station");
  append("SITE","Station");
  append("BODC_stn","BODC Station:METAVAR:INTEGER");
  append("Bot_Flag","Bottle Flag:TEXT:2");
  append("Firing_Seq","Firing Sequence:INTEGER");
  append("Bot_depth[metres]","DEPTH[m]");
  append("Bot_press[db]","PRESSURE[dbar]");
  append("BODC_bot","BODC Bottle Number:INTEGER");
  append("CELLTYPE","CELL_TYPE_BOTTLE:INDEXED_TEXT");
  if (cruise=="JC057" || cruise=="PE319" || cruise=="PE321")
    {
      append("Bot_Ref","Bottle Number:INTEGER");
      append("Rosette_Pos","GEOTRACES Sample Number:INTEGER");
    }
  else
    {
      append("Rosette_Pos","Bottle Number:INTEGER");
      append("Bot_Ref","GEOTRACES Sample Number:INTEGER");
    }
  append("["," [");
}

/**************************************************************************/
QString Replacer::applyTo(const QString& string)
/**************************************************************************/
/*!

  \brief Applies all pre/post replacements to string \a string and
  returns the result.

*/
{
  int i,n=size(); QString s=string;

  for (i=0; i<n; ++i)
    if (s.contains(pre.at(i))) s=s.replace(pre.at(i),post.at(i));

  return s.simplified();
}

/**************************************************************************/
QString Replacer::applyToEx(const QString& string)
/**************************************************************************/
/*!

  \brief Applies all pre/post replacements to string \a string and
  also changes the ERROR prefix to STANDARD_DEV.

  \return The modified string.
*/
{
  /* apply the replacer */
  QString s=applyTo(string);
 
  /* properly rename the error colomn labels */
  s=s.simplified();
  if (s.startsWith("ERROR"))
    s=QString("STANDARD_DEV::")+s.mid(s.indexOf("_")+1);

  return s;
}

/**************************************************************************/
void Replacer::applyTo(QStringList& sl)
/**************************************************************************/
/*!

  \brief Applies all pre/post replacements to all strings in \a sl.

*/
{
  int i,n=sl.size();
  for (i=0; i<n; ++i)
    sl[i]=applyToEx(sl.at(i));
}

/**************************************************************************/
void Replacer::convertPreEntriesToLower()
/**************************************************************************/
/*!

  \brief Converts all pre entries to lower case.

*/
{
  int i,n=pre.size();
  for (i=0; i<n; ++i)
    pre[i]=pre.at(i).toLower();
}

/**************************************************************************/
void Replacer::prepend(const QString& preString,const QString& postString)
/**************************************************************************/
/*!

  \brief Prepends pre/post strings \a preString / \a postString.

*/
{
  pre.append(preString); post.append(postString);
}

/**************************************************************************/
QString Replacer::undoTo(const QString& string)
/**************************************************************************/
/*!

  \brief Reverts all pre/post replacements previously applied to
  string \a string and returns the result.

*/
{
  int i,n=size(); QString s=string;

  for (i=n-1; i>=0; --i)
    if (!post.at(i).isEmpty() && s.contains(post.at(i)))
      s=s.replace(post.at(i),pre.at(i));

  return s.simplified();
}
