#include "MyModel.h"

#include <stdio.h>

GLuint MyModel::_sharedVertexArray = 0;

MyModel::MyModel()
  :_diffuseTexture(-1)
{
}

void MyModel::print() const
{
  printf("Modèle '%s': ", _name.c_str());
  _meshGpuLocation.print();
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
