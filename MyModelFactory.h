#ifndef __MYMODELFACTORY_H__
#define __MYMODELFACTORY_H__

#include "GLResource.h"

#include "MyMeshGpuLocation.h"

#include <map>
#include <vector>
#include <string>

class MyModel;

namespace gk
{
  class Mesh;
}

typedef std::map<std::string, MyMeshGpuLocation> MeshGpuLocationMap;

class MyModelFactory
{
 private:

  static int _totalVertexCount;
  static GLuint _sharedVertexBuffer;

  static int _totalIndexCount;
  static GLuint _sharedIndexBuffer;

  static MeshGpuLocationMap _meshGpuLocations;

 public:

  static MyModel* createModel(const std::string& filename);

 private:

  static void bindSharedBuffers(bool bind);

  static MyMeshGpuLocation getMeshGpuLocation(gk::Mesh* mesh);
  static MyMeshGpuLocation getMeshGpuLocation(const std::string& filename);
};

#endif
