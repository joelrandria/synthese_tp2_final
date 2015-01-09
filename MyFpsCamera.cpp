#include "MyFpsCamera.h"

MyFpsCamera::MyFpsCamera()
  :_up(gk::Vector(0, 1, 0)),
   _front(gk::Vector(0, 0, -1)),
   _right(gk::Vector(1, 0, 0))
{
}
MyFpsCamera::MyFpsCamera(const gk::Point& position, const gk::Vector& up, const gk::Vector& front)
  :_position(position),
   _up(gk::Normalize(up)),
   _front(gk::Normalize(front))
{
  updateRight();
  updateTransforms();
}

void MyFpsCamera::pitch(float localDegreeAngle)
{
  gk::Transform r = gk::Rotate(localDegreeAngle, _right);

  _up = r(_up);
  _front = r(_front);

  updateTransforms();
}
void MyFpsCamera::yaw(float globalDegreeAngle)
{
  gk::Transform r = gk::RotateY(globalDegreeAngle);

  _up = r(_up);
  _right = r(_right);
  _front = r(_front);

  updateTransforms();
}

void MyFpsCamera::translate(const gk::Vector& translation)
{
  _position += translation;

  updateTransforms();
}
void MyFpsCamera::localTranslate(const gk::Vector& translation)
{
  _position += _viewToWorldTransform(translation);

  updateTransforms();
}

void MyFpsCamera::print() const
{
  printf("\r\n");

  printf("Up (length = %f): ", _up.Length());
  _up.print();
  printf("Front: (length = %f): ", _front.Length());
  _front.print();
  printf("Right: (length = %f): ", _right.Length());
  _right.print();

  printf("\r\n");

  printf("----- ViewToWorld matrix -----\r\n\r\n");
  _viewToWorldTransform.print();
  printf("------------------------------\r\n");

  printf("----- WorldToView matrix -----\r\n\r\n");
  _worldToViewTransform.print();
  printf("------------------------------\r\n");
}

void MyFpsCamera::updateRight()
{
  _right = gk::Cross(_front, _up);
}
void MyFpsCamera::updateTransforms()
{
  _viewToWorldTransform = gk::Transform(gk::Matrix4x4(_right.x, _up.x, -_front.x, _position.x,
						      _right.y, _up.y, -_front.y, _position.y,
						      _right.z, _up.z, -_front.z, _position.z,
						      0, 0, 0, 1));
  _worldToViewTransform = _viewToWorldTransform.inverse();
}
