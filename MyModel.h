#ifndef __MYMODEL_H__
#define __MYMODEL_H__

#include "Transform.h"
#include "GLResource.h"

#include "MyMeshGpuLocation.h"

#include <vector>
#include <string>

class MyModel
{
  friend class MyModelFactory;

 private:

  std::string _name;

  gk::Point _position;
  gk::Transform _modelToWorldTransform;

  MyMeshGpuLocation _meshGpuLocation;

  static GLuint _sharedVertexArray;

 public:

  MyModel();

  const std::string& name() const { return _name; }
  const MyMeshGpuLocation& meshGpuInfo() const { return _meshGpuLocation; }
  const gk::Transform& modelToWorldTransform() const { return _modelToWorldTransform; }

  void print() const;

  void setPosition(const gk::Point& position);

  static GLuint sharedVertexArray() { return _sharedVertexArray; }

 private:

  void updateTransforms();
};

#endif
