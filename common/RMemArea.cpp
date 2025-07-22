/****************************************************************************
**
** Copyright (C) 1993-2021 Reiner Schlitzer. All rights reserved.
**
** This file is part of Ocean Data View.
**
** Class implementation of:
**                           RMemArea
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "RMemArea.h"

#include <stdlib.h>

#include "globalFunctions.h"
#include "common/odv.h"

/***************************************************************************
** NEW CLASS
***************************************************************************/

/*!
  \class RMemArea

  \brief Simple memory container that services memory requests and
  automatically grows if necessary.

  The memory of RMemArea objects is organized in a potentially large
  number of memory blocks. Each memory block is identified by a
  unique block ID (positive, negative or zero int value) and a block
  info 8-byte integer specifying start (LOWINT of the block info
  value) and byte-size (HIGHINT) of the block. A block info value \c
  -1 indicates an invalid entry.

  A typical usage of RMemArea is to hold metadata and data values of a
  station with variable IDs used as block IDs.

  RMemArea holds block info information in two ways: (a) in array \a
  blockInfoArr holding blockInfoArrSize block info values for block
  IDs 0 to blockInfoArrSize-1 allowing very fast lookups, and (b) in
  QMap blockInfosById holding block info values by block id (arbitrary
  IDs).

  \note Pointers returned by the \c request(), \c data() and some other
  functions become invalid when the container is resized. You should
  never store these pointers, but instead inquire them with the \c
  data() function whenever needed.
*/

/**************************************************************************/
RMemArea::RMemArea()
/**************************************************************************/
/*!
  \brief Constructs an empty memory container.
*/
{
  blockInfoArr=NULL; blockInfoArrSize=0;
  d=NULL; totalBytes=0;
  clear();
}

/**************************************************************************/
RMemArea::RMemArea(int nBytes,int blockInfoArraySize)
/**************************************************************************/
/*!

  \brief Constructs a memory container of size \a nBytes and
  establishes a block info array of size \a blockInfoArraySize.

*/
{
  blockInfoArr=NULL; blockInfoArrSize=0;
  resizeBlockArray(blockInfoArraySize);
  totalBytes=((d=(unsigned char*)malloc(nBytes))) ? nBytes:0;
  clear();
}

/**************************************************************************/
RMemArea::RMemArea(const RMemArea& other)
/**************************************************************************/
/*!
  \brief Constructs a copy of \a other.
*/
{
  blockInfoArr=NULL; blockInfoArrSize=0;
  resizeBlockArray(other.blockInfoArrSize);
  memcpy(blockInfoArr,other.blockInfoArr,blockInfoArrSize*sizeof(qint64));

  totalBytes=((d=(unsigned char*)malloc(other.totalBytes))) ? other.totalBytes:0;
  if (totalBytes)
    {
      usedBytes=other.usedBytes; lastID=other.lastID;
      blockInfosById=other.blockInfosById; memcpy(d,other.d,usedBytes);
    }
  else clear();
}

/**************************************************************************/
RMemArea::~RMemArea()
/**************************************************************************/
{
  if (d) free(d);
  if (blockInfoArr) free(blockInfoArr);
}

/**************************************************************************/
void RMemArea::clear()
/**************************************************************************/
/*!
  \brief Releases all used memory.
*/
{
  if (blockInfoArr) initArray<qint64>(blockInfoArr,blockInfoArrSize,-1);
  blockInfosById.clear(); usedBytes=0; lastID=ODV::missINT32;
}

/**************************************************************************/
void* RMemArea::place(int id,int byteOffset,int nBytes)
/**************************************************************************/
/*!
  \brief Registers a request \a id and assigns a byte offset of \a
  byteOffset.

  \return A (void*) pointer to the beginning of the registered memory
  block, or 0, if the request would not fit into the current size of
  the container.

  This function can be used to define a fixed layout of data items
  within the memory area.

  \note Pointers returned by this and other functions become invalid
  when the container is resized. You should never store these
  pointers, but instead inquire them with the \c data() function
  whenever needed.
*/
{
  if (d && totalBytes>=(byteOffset+nBytes))
    {
      setBlockInfoValue(id,byteOffset,nBytes);
      return (void*)((unsigned char*)d+byteOffset);
    }
  else return 0;
}

/**************************************************************************/
void RMemArea::releaseLastRequest()
/**************************************************************************/
/*!
  \brief Releases the memory of the last request.
*/
{
  qint64 v=-1;
  if (lastID>-1 && (v=blockInfoValue(lastID))>-1)
    { usedBytes=LOWINT(v); releaseRequest(lastID); lastID=ODV::missINT32; }
}

/**************************************************************************/
void RMemArea::releaseRequest(int id)
/**************************************************************************/
/*!
  \brief Releases the request \a id.

  \note The memory of the request becomes in-accessible.
*/
{
  if (id>-1 && id<blockInfoArrSize) *(blockInfoArr+id)=-1;
  else                              blockInfosById.remove(id);
}

/**************************************************************************/
void* RMemArea::request(int nBytes)
/**************************************************************************/
/*!

  \brief Returns a (void*) pointer to the beginning of the container's
  memory of at least \a nBytes bytes, or 0, in case of errors.

  This function clears any previous memory requests and calls \c
  request(-1,nBytes).

  \note Pointers returned by this and other functions become invalid
  when the container is resized. You should never store these
  pointers, but instead inquire them with the \c data() function
  whenever needed.
*/
{
  clear();
  return request(-1,nBytes);
}

/**************************************************************************/
void* RMemArea::request(int id,int nBytes)
/**************************************************************************/
/*!

  \brief Returns a (void*) pointer to \a nBytes of memory, or \c NULL,
  in case of errors.

  If the current size of the container does not accomodate \a nBytes
  more bytes, it is resized to hold the requested number of bytes, or
  to 125% of its current size, whichever is greater.

  \note Pointers returned by this and other functions become invalid
  when the container is resized. You should never store these
  pointers, but instead inquire them with the \c data() function
  whenever needed.
*/
{
  int newSize=(usedBytes+nBytes)*sizeof(unsigned char),N,offset,incFac;

  /* resize d if not large enough to hold nItems more items */
  incFac=(nBytes>totalBytes) ? 3 : 5;
  N=qMax(usedBytes+incFac*nBytes,(int)(1.25*totalBytes));
  if (newSize>totalBytes && (d=(unsigned char*)realloc((void*)d,N))) totalBytes=N;

  /* define the pointer and reserve nItems items initialized with missUINT8 */
  if (d)
    {
      offset=usedBytes; setBlockInfoValue(id,offset,nBytes);
      usedBytes+=nBytes; lastID=id;
      return (void*)((unsigned char*)d+offset);
    }
  else return NULL;
}

/**************************************************************************/
void RMemArea::requestMulti(int firstID,int lastID,int nBytes)
/**************************************************************************/
/*!

  \brief Requests \a nBytes of memory for all IDs between (and
  including) \a firstID and \a lastID.

*/
{
  for (int id=firstID; id<=lastID; ++id)
    request(id,nBytes);
}

/**************************************************************************/
void* RMemArea::resize(int nBytes,int blockInfoArraySize)
/**************************************************************************/
/*!

  \brief Destroys the current container (if any) and creates a new one
  of size \a nBytes and with a block info array of size \a
  blockInfoArraySize.

  Returns a (void*) pointer to the \a nBytes of memory, or \c 0,
  in case of errors.

  \note Pointers returned by this and other functions become invalid
  when the container is resized. You should never store these
  pointers, but instead inquire them with the \c data() function
  whenever needed.
*/
{
  resizeBlockArray(blockInfoArraySize);
  if (d) { free(d); d=NULL; }
  clear();
  totalBytes=(nBytes && (d=(unsigned char*)malloc(nBytes))) ? nBytes : 0;
  return (void*)d;
}

/**************************************************************************/
void RMemArea::resizeBlockArray(int blockInfoArraySize)
/**************************************************************************/
/*!

  \brief Destroys the current block info array (if any) and creates a
  new one of size \a blockInfoArraySize.

  The block info array is initialized with \c -1 values.

*/
{
  /* free an existing block info array */
  if (blockInfoArr)
    { free(blockInfoArr); blockInfoArr=NULL; blockInfoArrSize=0; }

  /* set up a block info array for blockInfoArraySize entries */
  if (blockInfoArraySize>0)
    {
      blockInfoArrSize=blockInfoArraySize;
      blockInfoArr=(qint64*) malloc(blockInfoArrSize*sizeof(qint64));
    }

  /* initialize the block info array with invalid entries */
  initArray<qint64>(blockInfoArr,blockInfoArrSize,-1);
}

/**************************************************************************/
RMemArea &RMemArea::operator=(const RMemArea &other)
/**************************************************************************/
/*!

  \brief Assigns \a other to this memory area and returns a reference to it.

*/
{
  resizeBlockArray(other.blockInfoArrSize);
  memcpy(blockInfoArr,other.blockInfoArr,blockInfoArrSize*sizeof(qint64));
  totalBytes=(d=(unsigned char*)realloc(d,other.totalBytes)) ? other.totalBytes : 0;
  if (totalBytes)
    {
      usedBytes=other.usedBytes; lastID=other.lastID;
      blockInfosById=other.blockInfosById; memcpy(d,other.d,usedBytes);
    }
  else
    clear();
  return *this;
}
