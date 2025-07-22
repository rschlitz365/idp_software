#ifndef RCONFIG_H
#define RCONFIG_H

/****************************************************************************
 **
 ** Copyright (C) 1993-2025 Reiner Schlitzer. All rights reserved.
 **
 ** This file is part of Ocean Data View.
 **
 ** Class definition of:
 **                       RConfig
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

#include <QMap>
#include <QString>

#include "common/declspec.h"

class QByteArray;
class QStringList;

/**************************************************************************/
class DECLSPEC RConfig
/**************************************************************************/
{
  public:
  typedef QMap<QString,QString> ConfigGroup;

  RConfig(const QString &fn,const char* codecName=NULL,
          const QString &gname=QString());
  ~RConfig();

  void    clear();
  void    clearGroup();
  void    close() { groups.clear(); filename=""; }
  //!< Clears the contents and closes this RConfig object.
  bool    deleteEntry(const QString &key);
  QByteArray getByteArrayEntry(const QString &key,
                               const QByteArray &dflt=QByteArray());
  QString getEntry(const QString &key,const QString &dflt=QString());
  QString getFileEntry(const QString &key,const QString &dfltPath=QString(),
                       bool checkForExistence=true);
  double  getFloatEntry(const QString &key,double dflt=0.);
  int     getIntEntry(const QString &key,int dflt=-1);
  QStringList getListEntry(const QString &key,const QChar &sep);
  QStringList getListEntry(const QString &key,const QString &sep);
  QString getNonEmptyEntry(const QString &key,const QString &dflt);
  int     groupEntryCount();
  QStringList groupEntryKeys();
  QList<QString> groupNames();
  QMap<QString,ConfigGroup>::Iterator groupIterator() { return git; }
  bool    hasGroup(const QString &gname);
  void    refresh(const QString &gname=QString());
  int     replaceInValues(const QString &oldString,const QString &newString,
                          const QString &gname=QString());
  void    setEntry(const QString &key,const QString &value);
  void    setEntry(const QString &key,int n);
  void    setEntry(const QString &key,double f);
  void    setEntry(const QString &key,const QStringList &lst,const QChar &sep);
  void    setEntry(const QString &key,const QByteArray &a);
  void    setGroup(const QString &gname);
  void    write(const QString &fn=QString());

  protected:
  void    parse(const QString &line);
  void    read();

  QString filename;
  QMap<QString,ConfigGroup> groups;
  QString cgName;                          //!current group name
  QMap<QString,ConfigGroup>::Iterator git; //!iterator to current group
  bool isModified;                         //!modification indicator
  char codec[81]; //!codec used for reading and writing (default encoding if empty)
};

#endif	/* !RCONFIG_H */
