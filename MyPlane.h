#ifndef __MYPLANE_H__
#define __MYPLANE_H__

#include "Geometry.h"

#include <vector>
#include <stdio.h>

#define PLANE_BOTH_HALFSPACES		0
#define PLANE_POSITIVE_HALFSPACE	1
#define PLANE_NEGATIVE_HALFSPACE	2
#define PLANE_ALIGNED			3

class MyPlane
{
 private:

  gk::Point _p;
  gk::Normal _n;

 public:

  MyPlane(const gk::Point& p, const gk::Normal& n)
    :_p(p),
    _n(n)
  {
  }

  void print() const
  {
    printf("p = <%f, %f, %f>, n = <%f, %f, %f>\r\n",
	   _p.x, _p.y, _p.z,
	   _n.x, _n.y, _n.z);
  }

  int locate(const gk::Point& point) const
  {
    float dot;

    dot = gk::Dot(_n, gk::Vector(_p, point));

    if (dot > 0)
      return PLANE_POSITIVE_HALFSPACE;
    else if (dot < 0)
      return PLANE_NEGATIVE_HALFSPACE;
    else
      return PLANE_ALIGNED;
  }
  int locate(const std::vector<gk::Point>& points) const
  {
    uint i;
    int location;

    location = locate(points[0]);

    for (i = 1; i < points.size(); ++i)
    {
      if (locate(points[i]) != location)
	return PLANE_BOTH_HALFSPACES;
    }

    return location;
  }
};

#endif
