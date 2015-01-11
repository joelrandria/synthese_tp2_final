#ifndef __MYPOINTLIGHT_H__
#define __MYPOINTLIGHT_H__

#include "Geometry.h"

#include "GL/GLSLUniforms.h"

#include <stdio.h>

class MyPointLight
{
 public:

  gk::glsl::vec4 position;

  gk::glsl::vec4 color;
  float constant_attenuation;
  float linear_attenuation;
  float quadratic_attenuation;

  float specularity;

 MyPointLight(const gk::Point& position,
	      const gk::Vec3& color,
	      float constant_attenuation,
	      float linear_attenuation,
	      float quadratic_attenuation,
	      float specularity)
   :position(position.x, position.y, position.z, 1),
    color(color.x, color.y, color.z, 1),
    constant_attenuation(constant_attenuation),
    linear_attenuation(linear_attenuation),
    quadratic_attenuation(quadratic_attenuation),
    specularity(specularity)
    {
    }

  void print() const
  {
    printf("Position = <%f, %f, %f>, Color = <%f, %f, %f>, Kc = %f, Kl = %f, Kq = %f\r\n",
	   (float)position.x, (float)position.y, (float)position.z,
	   (float)color.x, (float)color.y, (float)color.z,
	   (float)constant_attenuation, (float)linear_attenuation, (float)quadratic_attenuation);
  }
};

#endif
