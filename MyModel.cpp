#include "MyModel.h"

#include <stdio.h>

GLuint MyModel::_sharedVertexArray = 0;

MyModel::MyModel()
  :_materialDiffuseColorEnabled(false),
   _materialDiffuseTextureEnabled(false),
   _materialDiffuseTexture(-1),
   _materialSpecularityBlending(0)
{
}

void MyModel::print() const
{
  printf("[Modèle '%s']\r\n", _name.c_str());
  printf(" Texture name = %d\r\n", _materialDiffuseTexture);
  printf(" Mesh info: "); _meshInfo.print();
  printf(" Bounding box = "); boundingBox().print();
}

void MyModel::setPosition(const gk::Point& position)
{
  _position = position;

  updateTransforms();
}
void MyModel::updateTransforms()
{
  _modelToWorldTransform = gk::Transform(gk::Matrix4x4(1, 0, 0, _position.x,
						       0, 1, 0, _position.y,
						       0, 0, 1, _position.z,
						       0, 0, 0, 1));
}
