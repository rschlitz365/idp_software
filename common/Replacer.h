#ifndef REPLACER_H
#define REPLACER_H

/****************************************************************************
**
** Copyright (C) 2017 Reiner Schlitzer. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QString>
#include <QStringList>

#include "common/RConfig.h"

/**************************************************************************/
class Replacer
/**************************************************************************/
/*!

  \brief Container holding pre/post replacement strings.

*/
{
public:
  Replacer() { }
  Replacer(const QString& preString,const QString& postString)
    { append(preString,postString); }

  void    append(RConfig& cf);
  void    append(const QString& preString,const QString& postString);
  void    appendStandardReplacements(const QString& cruise=QString());
  QString applyTo(const QString& string);
  QString applyToEx(const QString& string);
  void    applyTo(QStringList& sl);
  void    convertPreEntriesToLower();
  void    prepend(const QString& preString,const QString& postString);
  int     size() { return pre.size(); }
  QString undoTo(const QString& string);

private:
  QStringList pre;
  QStringList post;
};

#endif   // REPLACER_H
