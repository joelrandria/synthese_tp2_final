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
    UpdateRight();
    UpdateTransforms();
  }

  gk::Transform ViewProjectionTransform() const
  {
    return (_projectionTransform * _worldToViewTransform);
  }

  void Pitch(float localAngleDeg)
  {
    gk::Transform r = gk::Rotate(localAngleDeg, _right);

    _up = r(_up);
    _front = r(_front);

    UpdateTransforms();
  }
  void Yaw(float globalAngleDeg)
  {
    gk::Transform r = gk::RotateY(globalAngleDeg);

    _right = r(_right);
    _front = r(_front);

    UpdateTransforms();
  }
  void LocalTranslate(const gk::Vector& translation)
  {
    _position += _viewToWorldTransform(translation);

    UpdateTransforms();
  }

 private:

  void UpdateRight()
  {
    _right = gk::Normalize(gk::Cross(_front, _up));
  }
  void UpdateTransforms()
  {
    _viewToWorldTransform = gk::Transform(gk::Matrix4x4(_right.x, _up.x, -_front.x, _position.x,
							_right.y, _up.y, -_front.y, _position.y,
							_right.z, _up.z, -_front.z, _position.z,
							0, 0, 0, 1));
    _worldToViewTransform = _viewToWorldTransform.inverse();
  }

};

#endif
