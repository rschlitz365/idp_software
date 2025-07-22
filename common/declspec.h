#ifndef DECLSPEC_H
#define DECLSPEC_H

/****************************************************************************
**
** Copyright (C) 1993-2025 Reiner Schlitzer. All rights reserved.
**
** This file is part of Ocean Data View.
**
**  Define the DECLSPEC variable:
**       MAKEDLL: export symbol, on Windows: __declspec(dllexport)
**       USEDLL:  import symbol, on Windows: __declspec(dllimport)
**       otherwise:    (empty)
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGlobal>

#if   defined MAKEDLL
 #define DECLSPEC Q_DECL_EXPORT
#elif defined USEDLL
 #define DECLSPEC Q_DECL_IMPORT
#else
 #define DECLSPEC
#endif

#endif	/* !DECLSPEC_H */
