#ifndef __MYMODELFACTORY_H__
#define __MYMODELFACTORY_H__

#include "GLResource.h"

#include "MyMeshInfo.h"

#include <map>
#include <vector>
#include <string>

class MyModel;

namespace gk
{
  class Image;
  class Mesh;
}

typedef std::map<std::string, MyMeshInfo> MeshInfoMap;
typedef std::map<std::string, GLuint> TextureMap;

class MyModelFactory
{
 private:

  static int _totalVertexCount;
  static GLuint _sharedVertexBuffer;

  static int _totalIndexCount;
  static GLuint _sharedIndexBuffer;

  static MeshInfoMap _meshInfos;
  static TextureMap _textures;

 public:

  static MyModel* createModel(const std::string& meshFilename, const std::string& diffuseTextureFilename);

 private:

  static void bindSharedBuffers(bool bind);

  static void setMesh(MyModel* model, gk::Mesh* mesh);
  static void setMesh(MyModel* model, const std::string& filename);

  static void setTexture(MyModel* model, gk::Image* image);
  static void setTexture(MyModel* model, const std::string& filename);
};

#endif
