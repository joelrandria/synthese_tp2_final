#ifndef __MYPOINTLIGHT_H__
#define __MYPOINTLIGHT_H__

#include "Geometry.h"

#include <stdio.h>

struct MyPointLight
{
  gk::Vec4 position;
  gk::Vec4 color;
  float specularity;

  float constant_attenuation;
  float linear_attenuation;
  float quadratic_attenuation;

MyPointLight(const gk::Vec4& position,
	       const gk::Vec4& color,
	       float specularity,
	       float constant_attenuation,
	       float linear_attenuation,
	       float quadratic_attenuation)
:position(position),
    color(color),
    specularity(specularity),
    constant_attenuation(constant_attenuation),
    linear_attenuation(linear_attenuation),
    quadratic_attenuation(quadratic_attenuation)
{
}

    void print() const
    {
printf("Position = <%f, %f, %f>, Color = <%f, %f, %f>, Kc = %f, Kl = %f, Kq = %f\r\n",
	 position.x, position.y, position.z,
	 color.x, color.y, color.z,
	 constant_attenuation, linear_attenuation, quadratic_attenuation);
    }
};

#endif
