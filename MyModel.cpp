#include "MyModel.h"

#include <stdio.h>

gk::GLVertexArray* MyModel::_globalVao = 0;

void MyModel::print() const
{
  printf("Modèle '%s': index count = %d, index offset = %d, vertex offset = %d\r\n",
	 _name.c_str(),
	 _indexCount,
	 _indexOffset,
	 _vertexOffset);
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
