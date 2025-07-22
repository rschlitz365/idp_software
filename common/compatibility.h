#ifndef COMPATIBILITY_H
#define COMPATIBILITY_H

#include <QtGlobal>

#if defined Q_OS_WIN

#include <windows.h>
#include <float.h>		/* for _finite, _copysign */

/****************************************************************************
 PLATFORM SPECIFIC HELPERS
****************************************************************************/
#define COPYSIGN _copysign

#else  /* not Q_OS_WIN */
/* typedefs and defines to be included on non-Windows systems */

/****************************************************************************
 DEBUGGING FLAGS
****************************************************************************/
#define _CRTDBG_ALLOC_MEM_DF        0x01  /* Turn on debug allocation */
#define _CRTDBG_DELAY_FREE_MEM_DF   0x02  /* Don't actually free memory */
#define _CRTDBG_CHECK_ALWAYS_DF     0x04  /* Check heap every alloc/dealloc */
#define _CRTDBG_RESERVED_DF         0x08  /* Reserved - do not use */
#define _CRTDBG_CHECK_CRT_DF        0x10  /* Leak check/diff CRT blocks */
#define _CRTDBG_LEAK_CHECK_DF       0x20  /* Leak check at program exit */

/****************************************************************************
 GENERAL
****************************************************************************/

#define _MAX_PATH   260 /* max. length of full pathname */
#define _MAX_DRIVE  3   /* max. length of drive component */
#define _MAX_DIR    256 /* max. length of path component */
#define _MAX_FNAME  256 /* max. length of file name component */
#define _MAX_EXT    256 /* max. length of extension component */

/* Pen Styles */
#define PS_SOLID            0
#define PS_DASH             1       /* -------  */
#define PS_DOT              2       /* .......  */
#define PS_DASHDOT          3       /* _._._._  */
#define PS_DASHDOTDOT       4       /* _.._.._  */

/****************************************************************************
 PLATFORM SPECIFIC HELPERS
****************************************************************************/
#define COPYSIGN copysign

#endif	/* not Q_OS_WIN */


#endif /* COMPATIBILITY_H */
