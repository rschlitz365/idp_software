#ifndef SYSTEMTOOLS_H
#define SYSTEMTOOLS_H

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
  This is the header file for functions related to the operating system.
*/

#include <QString>
#include <QStringList>

#include "common/declspec.h"

class QByteArray;

/*
 * T qbswap(T source).
 * Changes the byte order of a value from big endian to little endian or vice versa.
 * This function can be used if you are not concerned about alignment issues,
 * and it is therefore a bit more convenient and in most cases more efficient.
 */
template <typename T> Q_DECL_CONSTEXPR T qbswap(T source);

// These definitions are written so that they are recognized by most compilers
// as bswap and replaced with single instruction builtins if available.
template <> inline Q_DECL_CONSTEXPR quint64 qbswap<quint64>(quint64 source)
{
  return 0
    | ((source & Q_UINT64_C(0x00000000000000ff)) << 56)
    | ((source & Q_UINT64_C(0x000000000000ff00)) << 40)
    | ((source & Q_UINT64_C(0x0000000000ff0000)) << 24)
    | ((source & Q_UINT64_C(0x00000000ff000000)) << 8)
    | ((source & Q_UINT64_C(0x000000ff00000000)) >> 8)
    | ((source & Q_UINT64_C(0x0000ff0000000000)) >> 24)
    | ((source & Q_UINT64_C(0x00ff000000000000)) >> 40)
    | ((source & Q_UINT64_C(0xff00000000000000)) >> 56);
}

template <> inline Q_DECL_CONSTEXPR quint32 qbswap<quint32>(quint32 source)
{
  return 0
    | ((source & 0x000000ff) << 24)
    | ((source & 0x0000ff00) << 8)
    | ((source & 0x00ff0000) >> 8)
    | ((source & 0xff000000) >> 24);
}

template <> inline Q_DECL_CONSTEXPR quint16 qbswap<quint16>(quint16 source)
{
  return quint16( 0
                  | ((source & 0x00ff) << 8)
                  | ((source & 0xff00) >> 8) );
}

template <> inline Q_DECL_CONSTEXPR quint8 qbswap<quint8>(quint8 source)
{
  return source;
}

DECLSPEC
double*	doubleFromFloat(float *f,int n);
DECLSPEC
QString	hostName();
DECLSPEC
int	idealThreadCount();
DECLSPEC
QByteArray passwordHashFor(quint32 u);
DECLSPEC
QByteArray passwordHashFor(const QString &str);
DECLSPEC
int	spawnDetachedProcess(const QString& program,
                         const QStringList& args=QStringList(),
                         const QString& initialDir=QString());
DECLSPEC
int	spawnQuietProcess(const QString& cmdLine,
                      const QStringList& parameters,
                      const char *szInput,const QString& stdOutFilePath,
                      const QString& initialDir);
DECLSPEC
void	swapBytes2(unsigned char *buf,size_t nItems);
DECLSPEC
void	swapBytes4(unsigned char *buf,size_t nItems);
DECLSPEC
void	swapBytes8(unsigned char *buf,size_t nItems);
DECLSPEC
void	swapBytes(unsigned char *buf,size_t nItems,size_t itemBytes);
DECLSPEC
QString	userName();

#endif	/* !SYSTEMTOOLS_H */
