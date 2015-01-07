#ifndef __MYMODELFACTORY_H__
#define __MYMODELFACTORY_H__

#include "GLResource.h"

#include <vector>
#include <string>

class MyModel;

namespace gk
{
  class Mesh;
}

class MyModelFactory
{
 private:

  static int _totalVertexCount;
  static GLuint _sharedVertexBuffer;

  static int _totalIndexCount;
  static GLuint _sharedIndexBuffer;

 private:

  static void bindSharedBuffers(bool bind);

 public:

  static MyModel* createModel(gk::Mesh* mesh);
  static MyModel* createModel(const std::string& filename);
};

#endif
