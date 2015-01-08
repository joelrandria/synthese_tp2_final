#ifndef __MYMODEL_H__
#define __MYMODEL_H__

#include "Geometry.h"
#include "Transform.h"
#include "GLResource.h"

#include "MyMeshInfo.h"

#include <vector>
#include <string>

class MyModel
{
  friend class MyModelFactory;

 private:

  std::string _name;

  gk::Point _position;
  gk::Transform _modelToWorldTransform;

  static GLuint _sharedVertexArray;

  MyMeshInfo _meshInfo;

  int _diffuseTexture;

 public:

  MyModel();

  const std::string& name() const { return _name; }

  const MyMeshInfo& meshInfo() const { return _meshInfo; }

  bool hasDiffuseTexture() const { return _diffuseTexture >= 0; }
  GLuint diffuseTexture() { return (GLuint)_diffuseTexture; }

  const gk::Transform& modelToWorldTransform() const { return _modelToWorldTransform; }

  void print() const;

  void setPosition(const gk::Point& position);

  static GLuint sharedVertexArray() { return _sharedVertexArray; }

 private:

  void updateTransforms();
};

#endif
