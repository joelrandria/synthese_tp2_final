#ifndef __MYMODEL_H__
#define __MYMODEL_H__

#include "Mesh.h"
#include "Transform.h"

#include "Geometry.h"

#include "GL/GLVertexArray.h"

#include <vector>
#include <string>

class MyModel
{
 private:

  std::string _name;

  gk::Point _position;
  gk::Transform _modelToWorldTransform;

  int _indexCount;
  int _indexOffset;
  int _vertexOffset;

  gk::GLVertexArray* _vao;

  static std::vector<gk::GLVertexArray*> _vaos;

 public:

  gk::GLVertexArray* vao() const { return _vao; }
  const std::string& name() const { return _name; }

  int indexCount() const { return _indexCount; }
  int indexOffset() const { return _indexOffset; }
  int vertexOffset() const { return _vertexOffset; }

  const gk::Transform& modelToWorldTransform() const { return _modelToWorldTransform; }

  void print() const;

  void setPosition(const gk::Point& position);

  static std::vector<MyModel*> loadSharedVertexArrayModels(const std::vector<gk::Mesh*>& mesh);
  static std::vector<MyModel*> loadSharedVertexArrayModels(const std::vector<std::string>& filenames);

 private:

  void updateTransforms();

};

#endif
