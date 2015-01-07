#ifndef __MYMESHGPULOCATION_H__
#define __MYMESHGPULOCATION_H__

#include <stdio.h>

class MyMeshGpuLocation
{
 public:

  int indexCount;
  int indexOffset;
  int vertexOffset;

 public:

  MyMeshGpuLocation()
    :indexCount(0),
    indexOffset(0),
    vertexOffset(0)
  {
  }
  MyMeshGpuLocation(const MyMeshGpuLocation& instance)
  {
    *this = instance;
  }

  MyMeshGpuLocation& operator=(const MyMeshGpuLocation& instance)
  {
    indexCount = instance.indexCount;
    indexOffset = instance.indexOffset;
    vertexOffset = instance.vertexOffset;

    return *this;
  }

  void print() const
  {
    printf("index count = %d, index offset = %d, vertex offset = %d\r\n", indexCount, indexOffset, vertexOffset);
  }
};

#endif
