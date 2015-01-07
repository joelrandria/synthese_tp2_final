#ifndef __MYFPSCAMERA_H__
#define __MYFPSCAMERA_H__

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

  MyFpsCamera(const gk::Point& position, const gk::Vector& up, const gk::Vector& front);

  gk::Transform viewTransform() const { return _worldToViewTransform; }
  gk::Transform projectionTransform() const { return _projectionTransform; }

  void pitch(float localDegreeAngle);
  void yaw(float globalDegreeAngle);

  void translate(const gk::Vector& translation);
  void localTranslate(const gk::Vector& translation);

  void print() const;

 private:

  void updateRight();
  void updateTransforms();

};

#endif
