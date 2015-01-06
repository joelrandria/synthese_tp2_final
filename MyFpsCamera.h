#ifndef __MYFPSCAMERA_H__
#define __MYFPSCAMERA_H__

//#include "Geometry.h"
#include "Transform.h"

class MyFpsCamera
{
 private:

  gk::Point _position;

  gk::Vector _up;
  gk::Vector _front;
  gk::Vector _right;

  gk::Transform _worldToViewTransform;
  gk::Transform _viewToWorldTransform;

  gk::Transform _projectionTransform;

 public:

  MyFpsCamera(const gk::Point& position, const gk::Vector& up, const gk::Vector& front)
    :_position(position),
    _up(gk::Normalize(up)),
    _front(gk::Normalize(front)),
    _projectionTransform(gk::Perspective(60, 1, 0.01f, 1000))
  {
    updateRight();
    updateTransforms();
  }

  gk::Transform projectionViewTransform() const
  {
    return (_projectionTransform * _worldToViewTransform);
  }

  void pitch(float localAngleDeg)
  {
    gk::Transform r = gk::Rotate(localAngleDeg, _right);

    _up = r(_up);
    _front = r(_front);

    updateTransforms();
  }
  void yaw(float globalAngleDeg)
  {
    gk::Transform r = gk::RotateY(globalAngleDeg);

    _up = r(_up);
    _right = r(_right);
    _front = r(_front);

    updateTransforms();
  }

  void translate(const gk::Vector& translation)
  {
    _position += translation;

    updateTransforms();
  }
  void localTranslate(const gk::Vector& translation)
  {
    _position += _viewToWorldTransform(translation);

    updateTransforms();
  }

  void print() const
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

 private:

  void updateRight()
  {
    _right = gk::Cross(_front, _up);
  }
  void updateTransforms()
  {
    _viewToWorldTransform = gk::Transform(gk::Matrix4x4(_right.x, _up.x, -_front.x, _position.x,
							_right.y, _up.y, -_front.y, _position.y,
							_right.z, _up.z, -_front.z, _position.z,
							0, 0, 0, 1));
    _worldToViewTransform = _viewToWorldTransform.inverse();
  }

};

#endif
