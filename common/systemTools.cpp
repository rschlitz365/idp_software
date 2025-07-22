/****************************************************************************
 **
 ** Copyright (C) 1993-2025 Reiner Schlitzer. All rights reserved.
 **
 ** This file is part of Ocean Data View.
 **
 ** Global functions: functions related to the operating system
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

/*! \file
  This file implements functions related to the operating system.
*/
#include <QtGlobal>

#ifdef Q_OS_WIN
#include <Windows.h>
#include <Lmcons.h>
#else
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#endif

#include "common/systemTools.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QThread>

/**************************************************************************/
double* doubleFromFloat(float *f,int n)
/**************************************************************************/
/*!
  \brief Allocates a memory area for \a n double values and fills these
  with values from \a f.

  \return A pointer to the allocated double area.

  \note The caller is responsible for deleting the double area using
  \c delete[] when no longer needed.
*/
{
  double *d=new double[n]; if (d==NULL) return d;

  for (int i=0; i<n; ++i,++f)
    { d[i]=*f; }

  return d;
}

/**************************************************************************/
QString hostName()
/**************************************************************************/
/*!
  \return The host name determined automatically from the system.
*/
{
  static QString strHostName;

  if (strHostName.isEmpty())
    {
#if defined Q_OS_WIN
#if defined(UNICODE)
      {
        TCHAR winHostName[MAX_COMPUTERNAME_LENGTH+1];
        DWORD winHostNameSize=MAX_COMPUTERNAME_LENGTH+1;
        if (GetComputerName(winHostName,&winHostNameSize))
          strHostName=QString::fromWCharArray(winHostName);
        else
          strHostName=QString("unknown_host");
      }
#else
      {
        char winHostName[MAX_COMPUTERNAME_LENGTH+1];
        DWORD winHostNameSize=MAX_COMPUTERNAME_LENGTH+1;
        if (GetComputerNameA(winHostName,&winHostNameSize))
          strHostName=QString::fromLocal8Bit(winHostName);
        else
          strHostName=QString("unknown_host");
      }
#endif // UNICODE
#else
      char hostName[_POSIX_HOST_NAME_MAX+1];
      size_t hostNameSize=_POSIX_HOST_NAME_MAX+1;
      if (gethostname(hostName,hostNameSize)==0)
        strHostName=QString(hostName);
      else
        strHostName=QString("unknown_host");
#endif // Q_OS_WIN

      //  Qt variant:
      //       if ((strHostName=QHostInfo::localHostName()).isEmpty())
    }

  return strHostName;
}

/**************************************************************************/
int idealThreadCount()
/**************************************************************************/
/*!
  \brief Returns the number of processors on the system.

  Uses QThread::idealThreadCount() function.
*/
{
  static int pc=0;

  if (pc==0) { pc=QThread::idealThreadCount(); if (pc<=0) pc=1; }

  return pc;
}

/**************************************************************************/
QByteArray passwordHashFor(quint32 u)
/**************************************************************************/
/*!

  \return The password hash for \a u.

*/
{
  QCryptographicHash ch(QCryptographicHash::Sha1);
  char szB[21]; sprintf(szB,"%d-%d",u,u+1);
  ch.addData(szB,(int) strlen(szB));
  return ch.result();
}

/**************************************************************************/
QByteArray passwordHashFor(const QString &str)
/**************************************************************************/
/*!

  \return The password hash for \a str.

*/
{
  QCryptographicHash ch(QCryptographicHash::Sha1);
  QString s=QString("%1-%2").arg(str).arg(str);
  QByteArray ba=s.toUtf8();
  ch.addData(ba.data(),ba.size());
  return ch.result();
}

/**************************************************************************/
int spawnDetachedProcess(const QString& program,
                         const QStringList& args,const QString& initialDir)
/**************************************************************************/
/*!
  \brief Spawns detached process specified in \a args.

  \return 0 if the process could be created normally, and 1 if the
  process could not be created.
*/
{
  QProcess qp;

  if (!initialDir.isEmpty()) qp.setWorkingDirectory(initialDir);
  qp.startDetached(program,args);
  qDebug("State: %i", qp.state());
  return 0;
}

/**************************************************************************/
int spawnQuietProcess(const QString& cmdLine,
                      const QStringList& parameters,
                      const char *szInput,const QString& stdOutFilePath,
                      const QString& initialDir)
/**************************************************************************/
/*!
  \brief Spawns process specified in \a cmdLine and waits for
  spawned process to finish.

  \return The value that is returned by the command interpreter. It
  returns the value 0 only if the command interpreter returns the
  value 0. A return value of -1 indicates an error, and errno is set
  to one of the following values:...
*/
{
  QProcess p;

  p.setWorkingDirectory(initialDir);
  p.setStandardOutputFile(stdOutFilePath);
  p.start(cmdLine,parameters);
  if (!p.waitForStarted()) return 1;
  if (szInput && *szInput) { p.write(szInput); p.closeWriteChannel(); }
  if (!p.waitForFinished()) return 2;
  return p.exitCode();
}

/**************************************************************************/
void swapBytes2(unsigned char *buf, size_t nItems)
/**************************************************************************/
/*!
  \brief Swap byte order of \a nItems 2 Byte items in \a buf.
*/
{
  quint16 *pInBuf=(quint16*)buf;

  for (; nItems--; ++pInBuf)
    *pInBuf=qbswap(*pInBuf);
}

/**************************************************************************/
void swapBytes4(unsigned char *buf, size_t nItems)
/**************************************************************************/
/*!
  \brief Swap byte order of \a nItems 4 Byte items in \a buf.
*/
{
  quint32 *pInBuf=(quint32*)buf;

  for (; nItems--; ++pInBuf)
    *pInBuf=qbswap(*pInBuf);
}

/**************************************************************************/
void swapBytes8(unsigned char *buf, size_t nItems)
/**************************************************************************/
/*!
  \brief Swap byte order of \a nItems 8 Byte items in \a buf.
*/
{
  quint64 *pInBuf=(quint64 *)buf;

  for (; nItems--; ++pInBuf)
    *pInBuf=qbswap(*pInBuf);
}

/**************************************************************************/
void swapBytes(unsigned char *buf, size_t nItems, size_t itemBytes)
/**************************************************************************/
/*!
  \brief Swaps byte order of \a nItems items in \a buf (\a itemBytes
  per item).
*/
{
  switch (itemBytes)
    {
    case 2:
      swapBytes2(buf,nItems);
      break;
    case 4:
      swapBytes4(buf,nItems);
      break;
    case 8:
      swapBytes8(buf,nItems);
      break;
    default:
      qWarning("Byte swapping of items with length %lu is not supported!",
               itemBytes);
    }
}

/**************************************************************************/
QString userName()
/**************************************************************************/
/*!
  \return The user name determined automatically from the system.
*/
{
  static QString strUserName;

  if (strUserName.isEmpty())
    {
#if defined Q_OS_WIN
#if defined(UNICODE)
      {
        TCHAR winUserName[UNLEN+1]; // UNLEN is defined in LMCONS.H
        DWORD winUserNameSize=sizeof(winUserName);
        GetUserName(winUserName,&winUserNameSize);
        strUserName=QString::fromWCharArray(winUserName );
      }
#else
      {
        char winUserName[UNLEN+1]; // UNLEN is defined in LMCONS.H
        DWORD winUserNameSize=sizeof(winUserName);
        GetUserNameA(winUserName,&winUserNameSize);
        strUserName=QString::fromLocal8Bit(winUserName);
      }
#endif // UNICODE
#else
      char *pszUser;
      if ((pszUser=getenv("ODVUSER")) || (pszUser=getenv("LOGNAME")) ||
          (pszUser=getenv("USER")) || (pszUser=getlogin()))
        strUserName=QString(pszUser);
      else
        strUserName=QString("unknown_user");
#endif // Q_OS_WIN
    }

  return strUserName;
}
