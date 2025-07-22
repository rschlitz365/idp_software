/****************************************************************************
 **
 ** Copyright (C) 1993-2025 Reiner Schlitzer. All rights reserved.
 **
 ** This file is part of Ocean Data View.
 **
 ** Class implementation of:
 **                           RConfig
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/
#include "common/RConfig.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QTextStream>

/***************************************************************************
 ** NEW CLASS
 ***************************************************************************/

/*!
  \class RConfig

  \brief Manages ini file settings.
*/

/**************************************************************************/
RConfig::RConfig(const QString &fn,const char* codecName,const QString &gname)
  : filename(fn),isModified(false)
/**************************************************************************/
/*!

  \brief Creates a RConfig object and reads contents from file \a fn.

  Codec \a codecName is used for reading and writing. The default
  encoding is used if \a codecName is NULL (the default).

  If \a gname is not empty on entry also sets the current group to \a
  gname.

*/
{
  /* remember the codec, if any */
  if (codecName) strcpy(codec,codecName);
  else *codec='\0';

  /* read the file */
  git=groups.end(); read();

  /* set the current group, if provided */
  if (!gname.isEmpty()) setGroup(gname);
}

/**************************************************************************/
RConfig::~RConfig()
/**************************************************************************/
/*!
 */
{
  if (isModified) write();
}

/**************************************************************************/
void RConfig::clear()
/**************************************************************************/
/*!
  \brief Clears the entire contents of the RConfig object.
*/
{
  QMap<QString,ConfigGroup>::Iterator it=groups.begin();
  while (it!=groups.end())
    { (*it).clear(); ++it; }
  groups.clear(); git=groups.end(); cgName.clear();
}

/**************************************************************************/
void RConfig::clearGroup()
/**************************************************************************/
/*!
  \brief Clears the contents of the current group.
*/
{
  if (git==groups.end()) { qWarning("no group set"); return; }
  (*git).clear(); isModified=true;
}

/**************************************************************************/
bool RConfig::deleteEntry(const QString &key)
/**************************************************************************/
/*!
  \brief Deletes the entry with key \a key in the current group.

  \return \c true if entry \a key was found and deleted and \c false
  otherwise.

*/
{
  if (git==groups.end()) return false;
  if ((*git).find(key)!=(*git).end())
    { (*git).remove(key); isModified=true; return true; }
  else
    return false;
}

/**************************************************************************/
QByteArray RConfig::getByteArrayEntry(const QString &key,const QByteArray &dflt)
/**************************************************************************/
/*!

  \brief Retrieves the byte array associated with key \a key in the
  current group, or \a dflt if there is no such key in the current
  group or the current group is not defined.

  \note Byte arrays in settings files are stored in hexadecimal form.

*/
{
  QString s=getEntry(key);
  if (s.isEmpty()) return dflt;
  else             return QByteArray::fromHex(s.toLocal8Bit());
}

/**************************************************************************/
QString RConfig::getEntry(const QString &key,const QString &dflt)
/**************************************************************************/
/*!

  \brief Retrieves the string associated with key \a key in the
  current group, or \a dflt if there is no such key in the current
  group or the current group is not defined.

*/
{
  if (git==groups.end()) return dflt.trimmed();

  ConfigGroup::Iterator it=(*git).find(key);
  if (it!=(*git).end()) return (*it).trimmed();
  else                  return dflt.trimmed();
}

/**************************************************************************/
QString RConfig::getFileEntry(const QString &key,const QString &dfltPath,
                              bool checkForExistence)
/**************************************************************************/
/*!

  \brief Retrieves the file or directory path associated with key \a
  key in the current group, or \a dfltPath if there is no such key in
  the current group or the current group is not defined.

  If \a checkForExistence is \c true (the default), checks whether the
  retrieved file or directory path exists and returns \a dfltPath if
  this test fails.

  \note The returned file name uses '/' as file separator.

*/
{
  QString dflt=QDir::fromNativeSeparators(dfltPath.trimmed());
  if (git==groups.end()) return dflt;

  ConfigGroup::Iterator it=(*git).find(key);
  if (it!=(*git).end())
    {
      QString path=QDir::fromNativeSeparators((*it).trimmed());
      if (checkForExistence && !QFileInfo(path).exists()) path=dflt;
      return path;
    }
  else return dflt;
}

/**************************************************************************/
double RConfig::getFloatEntry(const QString &key,double dflt)
/**************************************************************************/
/*!
 */
{
  QString s=getEntry(key);
  if (s.isEmpty()) return dflt;
  else             return s.toDouble();
}

/**************************************************************************/
int RConfig::getIntEntry(const QString &key,int dflt)
/**************************************************************************/
/*!
 */
{
  QString s=getEntry(key);
  if (s.isEmpty()) return dflt;
  else             return s.toInt();
}

/**************************************************************************/
QStringList RConfig::getListEntry(const QString &key,const QChar &sep)
/**************************************************************************/
/*!
 */
{
  QString s=getEntry(key);
  if (s.isEmpty()) return QStringList();
  else             return s.split(sep);
}

/**************************************************************************/
QStringList RConfig::getListEntry(const QString &key,const QString &sep)
/**************************************************************************/
/*!
 */
{
  QString s=getEntry(key);
  if (s.isEmpty()) return QStringList();
  else             return s.split(sep);
}

/**************************************************************************/
QString RConfig::getNonEmptyEntry(const QString &key,const QString &dflt)
/**************************************************************************/
/*!

  \brief Retrieves the string associated with key \a key in the
  current group, or \a dflt if there is no such key in the current
  group, the current group is not defined, or the string associated
  with key \a key is empty.

*/
{
  QString s=getEntry(key,dflt);
  return s.isEmpty() ? dflt:s;
}

/**************************************************************************/
int RConfig::groupEntryCount()
/**************************************************************************/
/*!

  \brief Returns the number of entries in the current group, or 0 if
  there is no current group.

*/
{
  return (git==groups.end()) ? 0:(*git).count();
}

/**************************************************************************/
QStringList RConfig::groupEntryKeys()
/**************************************************************************/
/*!

  \brief Returns the key names of all entries in the current group, or
  an empty list if there is no current group.

*/
{
  return (git==groups.end()) ? QStringList():(*git).keys();
}

/**************************************************************************/
QList<QString> RConfig::groupNames()
/**************************************************************************/
/*!

  \brief Returns the list of group names.

*/
{
  return groups.keys();
}

/**************************************************************************/
bool RConfig::hasGroup(const QString &gname)
/**************************************************************************/
/*!
  \brief Return \c true if a group \a gname exists and \c false otherwise.
*/
{
  QMap<QString,ConfigGroup>::Iterator it=groups.find(gname);
  return (it==groups.end()) ? false:true;
}

/**************************************************************************/
void RConfig::parse(const QString &l)
/**************************************************************************/
/*!
  \brief Interpretes one line from the file.
*/
{
  QString line=l.trimmed(),key,value;

  if (line[0]==QChar('['))
    {
      /* line starts a new group [group_name] */
      QString gname=line; gname=gname.remove(0,1);
      if (gname[(int)gname.length()-1]==QChar(']'))
        gname=gname.remove(gname.length()-1,1);
      ConfigGroup *grp=new ConfigGroup;
      git=groups.insert(gname,*grp);
    }
  else
    {
      if (git==groups.end())
        {
          // qWarning("line `%s' out of group",line.toLocal8Bit().data());
          return;
        }
      int n=line.indexOf('=');
      if (n==-1)
        {
          // qWarning("corrupted line `%s' in group `%s'",
          // 	   line.toLocal8Bit().data(),git.key().toLocal8Bit().data());
          return;
        }
      key=line.left(n).trimmed();
      value=line.mid(n+1).trimmed();
      (*git).insert(key,value);
    }
}

/**************************************************************************/
void RConfig::read()
/**************************************************************************/
/*!
 */
{
  QFile f(filename);
  if (!f.exists() || !f.open(QIODevice::Text | QIODevice::ReadOnly))
    { git=groups.end(); return; }

  QTextStream s(&f); QString line;

  /* set the proper encoding if requested */
  if (strlen(codec)) s.setCodec(codec);

  /* read and parse the file */
  while (!s.atEnd()) { line=s.readLine(); parse(line); }

  f.close(); isModified=false;
}

/**************************************************************************/
void RConfig::refresh(const QString &gname)
/**************************************************************************/
/*!

  \brief Clears and re-reads the contents from the file.

  Sets the current group to \a gname, or, if \a gname is empty (the
  default), to the previous current group.

*/
{
  QString gn=gname; if (gn.isEmpty() && git!=groups.end()) gn=cgName;
  clear(); read(); if (!gn.isEmpty()) setGroup(gn);
}

/**************************************************************************/
int RConfig::replaceInValues(const QString &oldString,const QString &newString,
                             const QString &gname)
/**************************************************************************/
/*!

  \brief Iterates over all entries of group \a gname (or all groups,
  if \a gname is empty on entry, the default) and replaces in the
  values \a oldString with \a newString.

  \return The number of changed entries.

*/
{
  int n=0;

  if (gname.isEmpty())
    {
      /* iterate over all groups */
      QMap<QString,ConfigGroup>::Iterator g_it=groups.begin();
      while (g_it!=groups.end())
        { n+=replaceInValues(oldString,newString,g_it.key()); ++g_it; }
    }
  else
    {
      /* iterate over all entries of group gname */
      QMap<QString,ConfigGroup>::Iterator g_it=groups.find(gname);
      if (g_it!=groups.end())
        {
          ConfigGroup::Iterator e_it=(*g_it).begin(); QString k,v;
          for (; e_it!=(*g_it).end(); ++e_it)
            {
              k=e_it.key(); v=*e_it;
              if (v.contains(oldString))
                { v.replace(oldString,newString); (*g_it).insert(k,v); ++n; }

            }
        }
    }

  /* set the modified flag if changes were made */
  if (n>0) isModified=true;

  return n;
}

/**************************************************************************/
void RConfig::setEntry(const QString &key,const QString &value)
/**************************************************************************/
/*!
 */
{
  if (git==groups.end()) { qWarning("no group set"); return; }
  (*git).insert(key,value); isModified=true;
}

/**************************************************************************/
void RConfig::setEntry(const QString &key,int n)
/**************************************************************************/
/*!
 */
{
  setEntry(key,QString::number(n));
}

/**************************************************************************/
void RConfig::setEntry(const QString &key,double f)
/**************************************************************************/
/*!
 */
{
  setEntry(key,QString::number(f));
}

/**************************************************************************/
void RConfig::setEntry(const QString &key,
                       const QStringList &lst,const QChar &sep)
/**************************************************************************/
/*!
 */
{
  QString s; QStringList::ConstIterator it=lst.begin();
  for (; it!=lst.end(); ++it) { s+=*it+sep; }
  setEntry(key,s);
}

/**************************************************************************/
void RConfig::setEntry(const QString &key,const QByteArray &a)
/**************************************************************************/
/*!
 */
{
  setEntry(key,QString(a.toHex()));
}

/**************************************************************************/
void RConfig::setGroup(const QString &gname)
/**************************************************************************/
/*!
  \brief Selects group \a gname as current group.

  The group \a gname is created if it does not exist.
*/
{
  QMap<QString,ConfigGroup>::Iterator it=groups.find(gname);
  if (it==groups.end())
    {
      ConfigGroup *grp=new ConfigGroup;
      git=groups.insert(gname,*grp);
      return;
    }
  git=it; cgName=gname;
}

/**************************************************************************/
void RConfig::write(const QString &fn)
/**************************************************************************/
/*!
  \brief Writes the contents of this object to settings file \a fn.

  The object's filepath is used if \a fn is the NULL string on entry
  (the default).

*/
{
  /* immediate return if this object is empty */
  if (!groups.size()) return;

  if (!fn.isEmpty()) filename=fn;

  QFile f(filename);
  if (!f.open(QIODevice::Text | QIODevice::WriteOnly))
    {
      //qWarning("could not open for writing `%s'",qPrintable(filename));
      git=groups.end(); return;
    }

  /* create a textstream */
  QTextStream s(&f);

  /* set the proper encoding if requested */
  if (strlen(codec)) s.setCodec(codec);

  /* write the contents to textstream */
  QMap< QString,ConfigGroup >::Iterator g_it=groups.begin();
  for (; g_it!=groups.end(); ++g_it)
    {
      //++n; if (n>1) s << "\n";
      s << "\n[" << g_it.key() << "]" << "\n";
      ConfigGroup::Iterator e_it=(*g_it).begin();
      for (; e_it!=(*g_it).end(); ++e_it)
        { s << e_it.key() << " = " << *e_it << "\n"; }
    }

  f.close(); isModified=false;
}
