#ifndef ODVNAMESPACE_H
#define ODVNAMESPACE_H

/****************************************************************************
**
** Copyright (C) 1993-2025 Reiner Schlitzer. All rights reserved.
**
** This file is part of Ocean Data View.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QObject>
#include <QtGlobal>

#include "common/declspec.h"

/*! \file
  \brief This file provides globally used declarations.
*/

/*! \class ODV
  \brief Declarations for global use

  The ODV class contains declarations and definitions used
  throughout %ODV. This includes the return status of functions
  and constants.

  \ingroup DoxyGroupCollection
*/

/****************************************************************************
** GENERAL DECLARATIONS
****************************************************************************/

class DECLSPEC ODV
{
  Q_GADGET;
  Q_ENUMS(Status);

public:
  /*!  Status values returned by many functions. */
  enum Status
  {
    NoErr=0,              /*!< OK  */
    UserAbort,            /*!< Aborted by user  */
    EOD,                  /*!< End of data reached */
    CollReadOnly,         /*!< Collection not writable */
    UnknownFileTypeErr,   /*!< Unknown file type */
    NoSuchDirErr,         /*!< Directory does not exist */
    DirCreateErr,         /*!< Could not create directory */
    FileOpenErr,          /*!< Could not open file */
    FileReadErr,          /*!< Error on read in file */
    FileWriteErr,         /*!< Error on write to file */
    FileErr,              /*!< A general error on file operation occured */
    CollOpenErr,          /*!< Could not open collection */
    CollCreateErr,        /*!< Could not create collection */
    CollDelErr,           /*!< Could not delete collection */
    CollCopyErr,          /*!< Could not copy collection */
    CollFormatUnsupported,/*!< The collection format is not supported */
    CollReadErr,          /*!< Error on read in collection */
    CollRenameErr,        /*!< Could not rename collection */
    CollWriteErr,         /*!< Error on write to collection */
    CollNotSorted,        /*!< Could not sort & condense collection */
    ImportErr,            /*!< Error on import */
    ImportProblems,       /*!< Import was done but there were problems.
                               Probably not all data could be imported. */
    AnalyzeFileErr,       /*!< Error on analyzing file */
    RPostfixEvalErr,      /*!< Syntax error in postfix expression */
    VarNotFound,          /*!< A variable could not be identified */
    PSPreambleNotFound,   /*!< Postscript preamble file not found. */
    StatIDOutOfRange,     /*!< ID of requested station is out of range. */
    InvalidStationData,   /*!< Data of station is invalid or unreasonable. */
    OutOfMemory,          /*!< Machine is out of memory. */
    BadParameter,         /*!< A bad parameter value was supplied. */
    BadQfSchema,          /*!< A bad quality flag schema was supplied. */
    NotImplemented        /*!< The requested functionality is not implemented. */
  };

  /*! Processing action types */
  enum ActionType
  {
    CANCEL=0,
    KEEP,
    APPLY,
    APPLYTOALL
  };

  /*! File and collection access modes */
  enum AccessMode
  {
    NoAccess=0,          /*!< No access. */
    ReadOnly=1,          /*!< Read-only access. */
    ReadWrite=3          /*!< Read-write access. */
  };

  /*! Date text formats supported by %ODV. */
  enum DateFormat
  {
    IsoDate,		/*!< Date format according to ISO 8601. <br>
			     Example: <tt>2006-02-23</tt>  for Feb/23/2006 */
    mmddyyyyDate,	/*!< Date in one pass with separators. <br>
			     Example: <tt>02/23/2006</tt> for Feb/23/2006	*/
    mmmddyyyyDate,	/*!< Date with abbreviated english month name in front. <br>
			     Example: <tt>Feb 23 2006</tt> for Feb/23/2006 */
    ddmonthyyyyDate,	/*!< Date with full english month name. <br>
			     Example: <tt>23 February 2006</tt> for Feb/23/2006. */
    ddmmmyyyyDate,	/*!< Date with abbreviated english month name. <br>
			     Example: <tt>23 Feb 2006</tt> for Feb/23/2006 */
    yyyymmddDate	/*!< Date in one pass without separators. <br>
			     Example: <tt>20060223</tt> for Feb/23/2006	*/
  };

  /*! Time text formats supported by %ODV. */
  enum TimeFormat
  {
    IsoTime,	  /*!< Time format according to ISO 8601. <br>
	      	     Example: <tt>21:03:59</tt>  for 3 min and 59 sec after 9pm */
    hhmmssTime,	  /*!< Time format without separators. <br>
	      	     Example: <tt>210359</tt>  for 3 min and 59 sec after 9pm */
    hhmmTime,	  /*!< Time format without separators. <br>
	      	     Example: <tt>2103</tt>  for 3 min and 59 sec after 9pm */
  };

  /*!  Missing value constants for different data types. */
  static const qint8   missINT8;   /*!< Missing value for qint8 data type. */
  static const quint8  missUINT8;  /*!< Missing value for quint8 data type. */
  static const qint16  missINT16;  /*!< Missing value for qint16 data type. */
  static const quint16 missUINT16; /*!< Missing value for quint16 data type. */
  static const qint32  missINT32;  /*!< Missing value for qint32 data type. */
  static const quint32 missUINT32; /*!< Missing value for quint32 data type. */
  static const qint64  missINT64;  /*!< Missing value for qint64 data type. */
  static const quint64 missUINT64; /*!< Missing value for quint64 data type. */
  static const float   missFLOAT;  /*!< Missing value for float data type. */
  static const double  missDOUBLE; /*!< Missing value for double data type. */

  /*!  Large value constants for different data types. */
  static const qint8   largeINT8;  /*!< Large positive value for qint8 data type. */
  static const quint8  largeUINT8; /*!< Large positive value for quint8 data type. */
  static const qint16  largeINT16; /*!< Large positive value for qint16 data type. */
  static const quint16 largeUINT16;/*!< Large positive value for quint16 data type. */
  static const qint32  largeINT32; /*!< Large positive value for qint32 data type. */
  static const quint32 largeUINT32;/*!< Large positive value for quint32 data type. */
  static const qint64  largeINT64; /*!< Large positive value for qint64 data type. */
  static const quint64 largeUINT64;/*!< Large positive value for quint64 data type. */
  static const float   largeFLOAT; /*!< Large positive value for float data type. */
  static const double  largeDOUBLE;/*!< Large positive value for double data type. */

  /*!  Special value constants for different data types. */
  static const quint32 specialUINT32;/*!< Special value for quint32 data type. */

  /* Version for QDataStream binary files */
  static const qint32  STREAMVERSION;
};

/*! Definition for a not assigned variable ID  */
#define VID_NONE           ODV::missINT32

#endif // ODVNAMESPACE_H
