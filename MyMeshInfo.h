#ifndef __MYMESHINFO_H__
#define __MYMESHINFO_H__

#include "Geometry.h"

#include <stdio.h>

class MyMeshInfo
{
 public:

  int gpuIndexCount;
  int gpuIndexOffset;
  int gpuVertexOffset;

  gk::BBox localBoundingBox;

 public:

  MyMeshInfo()
    :gpuIndexCount(0),
    gpuIndexOffset(0),
    gpuVertexOffset(0)
  {
  }
  MyMeshInfo(const MyMeshInfo& instance)
  {
    *this = instance;
  }

  MyMeshInfo& operator=(const MyMeshInfo& instance)
  {
    gpuIndexCount = instance.gpuIndexCount;
    gpuIndexOffset = instance.gpuIndexOffset;
    gpuVertexOffset = instance.gpuVertexOffset;

    localBoundingBox = instance.localBoundingBox;

    return *this;
  }

  void print() const
  {
    printf("GPU index count = %d, GPU index offset = %d, GPU vertex offset = %d",
	   gpuIndexCount,
	   gpuIndexOffset,
	   gpuVertexOffset);
    printf(", Local bounding box = "); localBoundingBox.print();
  }
};

#endif
