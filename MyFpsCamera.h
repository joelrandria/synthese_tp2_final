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

  GLuint _framebuffer;
  int _renderingWidth;
  int _renderingHeight;

 public:

  MyFpsCamera(const gk::Point& position = gk::Point(),
	      const gk::Vector& up = gk::Vector(0, 1, 0),
	      const gk::Vector& front = gk::Vector(0, 0, -1),
	      const gk::Transform& projectionTransform = gk::Transform());

  gk::Transform viewTransform() const { return _worldToViewTransform; }

  gk::Transform getProjectionTransform() const { return _projectionTransform; }
  void setProjectionTransform(const gk::Transform& p) { _projectionTransform = p; }

  GLuint& framebuffer() { return _framebuffer; }
  int& renderingWidth() { return _renderingWidth; }
  int& renderingHeight() { return _renderingHeight; }

  void print() const;

  void pitch(float localDegreeAngle);
  void yaw(float globalDegreeAngle);

  void translate(const gk::Vector& translation);
  void localTranslate(const gk::Vector& translation);

  bool isVisible(const MyModel& model) const;

 private:

  void updateRight();
  void updateTransforms();

  void projectBoundingBox(gk::BBox bbox, std::vector<gk::Point>& ndcBBoxVertices) const;
};

#endif
