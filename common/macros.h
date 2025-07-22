#ifndef ODVMACROS_H
#define ODVMACROS_H

/****************************************************************************
**
** Copyright (C) 1993-2021 Reiner Schlitzer. All rights reserved.
**
** This file is part of Ocean Data View.
**
** Definition of helpful macros.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QMetaEnum>

/*! \file
  \brief This file declares some helpful macros to deal with certain enumerations.
*/

/*!
  This macro returns the QMetaEnum object for class \a o and enumeration object \a e.
  For more information on QMetaEnums see Qt documentation.
*/
#define ENUM_OBJ(o,e)	 (o::staticMetaObject.enumerator(o::staticMetaObject.indexOfEnumerator(#e)))

/*!
  This macro returns the name of an enumeration object member \a v.
  The class name \a o and the enumeration object name \a e must be specified too.
  \sa ENUM_OBJ()
*/
#define ENUM_NAME(o,e,v) (ENUM_OBJ(o,e).valueToKey((v)))

/*! HIGHINT(int64) returns the high-order 32 bit int of int64. */
#define HIGHINT(int64) (int64 >> 32)
/*! LOWINT(int64) returns the low-order 32 bit int of int64. */
#define LOWINT(int64) (int64 & 0xffffffff)
/*! MAKEINT64(lowInt,highInt) returns the 64 bit combination of lowInt and highInt. */
#define MAKEINT64(lowInt,highInt) ((qint64) highInt << 32 | lowInt)

#endif	/* !ODVMACROS_H */
