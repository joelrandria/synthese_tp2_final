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

  /*! Général */
  std::string _name;

  /*! Transformations */
  gk::Point _position;
  gk::Transform _modelToWorldTransform;

  /*! Maillage */
  static GLuint _sharedVertexArray;
  MyMeshInfo _meshInfo;

  /*! Matériau */
  bool _materialDiffuseColorEnabled;
  bool _materialDiffuseTextureEnabled;

  gk::Vec3 _materialDiffuseColor;
  GLuint _materialDiffuseTexture;

  float _materialSpecularity;
  float _materialSpecularityBlending;

 public:

  MyModel();

  const std::string& name() const { return _name; }

  const MyMeshInfo& meshInfo() const { return _meshInfo; }

  bool& materialDiffuseColorEnabled() { return _materialDiffuseColorEnabled; }
  bool& materialDiffuseTextureEnabled() { return _materialDiffuseTextureEnabled; }

  gk::Vec3& materialDiffuseColor() { return _materialDiffuseColor; }
  GLuint materialDiffuseTexture() const { return _materialDiffuseTexture; }

  float& materialSpecularity() { return _materialSpecularity; }
  float& materialSpecularityBlending() { return _materialSpecularityBlending; }

  const gk::Transform& modelToWorldTransform() const { return _modelToWorldTransform; }

  gk::BBox boundingBox() const { return _modelToWorldTransform(_meshInfo.localBoundingBox); }

  void print() const;

  void setPosition(const gk::Point& position);

  static GLuint sharedVertexArray() { return _sharedVertexArray; }

 private:

  void updateTransforms();
};

#endif
