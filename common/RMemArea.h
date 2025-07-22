#ifndef RMEMAREA_H
#define RMEMAREA_H

/****************************************************************************
**
** Copyright (C) 1993-2021 Reiner Schlitzer. All rights reserved.
**
** This file is part of Ocean Data View.
**
** Class definition of:
**                       RMemArea
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QMap>

#include "macros.h"

/**************************************************************************/
class RMemArea
/**************************************************************************/
{
private:
  qint64 blockInfoValue(int id) const
  {
    return (id>-1 && id<blockInfoArrSize) ?
      *(blockInfoArr+id) : blockInfosById.value(id,-1); 
  }

 public:
  RMemArea();
  RMemArea(int nBytes,int blockInfoArraySize);
  RMemArea(const RMemArea& other);
  ~RMemArea();

  int   byteOffset(int id) const
  { qint64 v=blockInfoValue(id); return (v>-1) ? LOWINT(v) : -1; }
  //!< Returns the Byte offset to the data of request \a id, or -1 if no such request
  int   byteSize(int id) const
  { qint64 v=blockInfoValue(id); return (v>-1) ? HIGHINT(v) : -1; }
  //!< Returns the Byte size to the data of request \a id, or -1 if no such request
  void  clear();
  void* data() { return (void*) d; }
  //!< Returns void* ptr to beginning of container data
  void* data(int id) const
  {
    qint64 v=blockInfoValue(id);
    return (v>-1) ? (void*)((unsigned char*)d+LOWINT(v)) : 0;
  }
  //!< Returns void* pointer to data of request \a id, or 0 if no such request
  void* place(int id,int byteOffset,int nBytes);
  void  releaseLastRequest();
  void  releaseRequest(int id);
  void* request(int nBytes=0);
  void* request(int id,int nBytes);
  void  requestMulti(int firstID,int lastID,int nBytes);
  void* resize(int nBytes,int blockInfoArraySize);
  int   size() const { return totalBytes; }
  //!< Returns the allocated bytes for the container.
  RMemArea &operator=(const RMemArea &other);

private:
  void resizeBlockArray(int blockInfoArraySize);
  void setBlockInfoValue(int id,int byteOffset,int nBytes)
  {
    if (id>-1 && id<blockInfoArrSize)
      *(blockInfoArr+id)=MAKEINT64(byteOffset,nBytes);
    else
      blockInfosById.insert(id,MAKEINT64(byteOffset,nBytes));
  }

  unsigned char *d; //!< Pointer to beginning of memory area.
  int totalBytes;   //!< Allocated bytes for d
  int usedBytes;    //!< Used bytes in d
  int lastID;       //!< ID of last request

  int blockInfoArrSize; //!< Size of the array of block infos blockInfoArr
  qint64 *blockInfoArr; //!< Array of block infos for ID range 0 to blockInfoArrSize-1
  QMap<int,qint64> blockInfosById;
  //!< QMap storing request IDs (key) and block infos (L: byte offset, H: byte size)
};

#endif	/* !RMEMAREA_H */
