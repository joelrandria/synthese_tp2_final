#ifndef __MYMODEL_H__
#define __MYMODEL_H__

#include "Transform.h"

#include "GL/GLVertexArray.h"

#include <vector>
#include <string>

namespace gk
{
  class GLVertexArray;
}

class MyModel
{
  friend class MyModelFactory;

 private:

  std::string _name;

  gk::Point _position;
  gk::Transform _modelToWorldTransform;

  int _indexCount;
  int _indexOffset;
  int _vertexOffset;

  static gk::GLVertexArray* _globalVao;

 public:

  const std::string& name() const { return _name; }

  int indexCount() const { return _indexCount; }
  int indexOffset() const { return _indexOffset; }
  int vertexOffset() const { return _vertexOffset; }

  const gk::Transform& modelToWorldTransform() const { return _modelToWorldTransform; }

  void print() const;

  void setPosition(const gk::Point& position);

  static gk::GLVertexArray* globalVao() { return _globalVao; }

 private:

  void updateTransforms();

};

#endif
