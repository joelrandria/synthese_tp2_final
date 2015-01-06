#ifndef __MYMODELFACTORY_H__
#define __MYMODELFACTORY_H__

#include <vector>
#include <string>

class MyModel;

namespace gk
{
  class Mesh;
}

class MyModelFactory
{
 public:

  static std::vector<MyModel*> createSharedVertexArrayModels(const std::vector<gk::Mesh*>& mesh);
  static std::vector<MyModel*> createSharedVertexArrayModels(const std::vector<std::string>& filenames);

};

#endif
