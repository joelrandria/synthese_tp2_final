#include "MyFpsCamera.h"
#include "MyModel.h"
#include "MyPlane.h"

#include <stdio.h>

const MyPlane _unitCubePlanes[6] = {
  MyPlane(gk::Point(1, 0, 0), gk::Normal(-1, 0, 0)),
  MyPlane(gk::Point(0, 0, 1), gk::Normal(0, 0, -1)),
  MyPlane(gk::Point(-1, 0, 0), gk::Normal(1, 0, 0)),
  MyPlane(gk::Point(0, 0, -1), gk::Normal(0, 0, 1)),
  MyPlane(gk::Point(0, -1, 0), gk::Normal(0, 1, 0)),
  MyPlane(gk::Point(0, 1, 0), gk::Normal(0, -1, 0))
};

MyFpsCamera::MyFpsCamera(const gk::Point& position,
			 const gk::Vector& up,
			 const gk::Vector& front,
			 const gk::Transform& projectionTransform)
  :_position(position),
   _up(gk::Normalize(up)),
   _front(gk::Normalize(front)),
   _projectionTransform(projectionTransform)
{
  updateRight();
  updateTransforms();
}

void MyFpsCamera::print() const
{
  printf("\r\n");

  printf("Position: "); _position.print();

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

bool MyFpsCamera::isVisible(const MyModel& model)
{
  uint i;
  std::vector<gk::Point> ndcBBoxVertices;

  projectBoundingBox(_worldToViewTransform(model.boundingBox()), ndcBBoxVertices);

  for (i = 0; i < 6; ++i)
    if (_unitCubePlanes[i].locate(ndcBBoxVertices) == PLANE_NEGATIVE_HALFSPACE)
      return false;

  return true;
}

void MyFpsCamera::projectBoundingBox(const gk::BBox& bbox, std::vector<gk::Point>& ndcBBoxVertices) const
{
  ndcBBoxVertices.clear();

  ndcBBoxVertices.push_back(_projectionTransform(gk::Point(bbox.pMin.x, bbox.pMin.y, bbox.pMin.z)));
  ndcBBoxVertices.push_back(_projectionTransform(gk::Point(bbox.pMax.x, bbox.pMin.y, bbox.pMin.z)));
  ndcBBoxVertices.push_back(_projectionTransform(gk::Point(bbox.pMax.x, bbox.pMax.y, bbox.pMin.z)));
  ndcBBoxVertices.push_back(_projectionTransform(gk::Point(bbox.pMin.x, bbox.pMax.y, bbox.pMin.z)));

  ndcBBoxVertices.push_back(_projectionTransform(gk::Point(bbox.pMin.x, bbox.pMin.y, bbox.pMax.z)));
  ndcBBoxVertices.push_back(_projectionTransform(gk::Point(bbox.pMax.x, bbox.pMin.y, bbox.pMax.z)));
  ndcBBoxVertices.push_back(_projectionTransform(gk::Point(bbox.pMax.x, bbox.pMax.y, bbox.pMax.z)));
  ndcBBoxVertices.push_back(_projectionTransform(gk::Point(bbox.pMin.x, bbox.pMax.y, bbox.pMax.z)));
}
