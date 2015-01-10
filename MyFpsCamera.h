#ifndef __MYFPSCAMERA_H__
#define __MYFPSCAMERA_H__

#include "MyModel.h"

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

  MyFpsCamera();
  MyFpsCamera(const gk::Point& position, const gk::Vector& up, const gk::Vector& front);

  gk::Transform viewTransform() const { return _worldToViewTransform; }
  gk::Transform& projectionTransform() { return _projectionTransform; }

  void print() const;

  void pitch(float localDegreeAngle);
  void yaw(float globalDegreeAngle);

  void translate(const gk::Vector& translation);
  void localTranslate(const gk::Vector& translation);

  bool isVisible(const MyModel& model);

 private:

  void updateRight();
  void updateTransforms();

  void projectBoundingBox(const gk::BBox& bbox, std::vector<gk::Point>& vertices) const;

};

#endif
