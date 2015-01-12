#ifndef __MYPOINTLIGHT_H__
#define __MYPOINTLIGHT_H__

#include "Geometry.h"
#include "Transform.h"

#include "GL/GLPlatform.h"
#include "GL/GLSLUniforms.h"

#include <vector>

class MyModel;

class MyPointLight
{
 public:

  gk::glsl::vec4 position;
  gk::glsl::vec4 color;

  float constant_attenuation;
  float linear_attenuation;
  float quadratic_attenuation;
  float specularity;

  GLuint framebuffer;

  MyPointLight(const gk::Point& position,
	       const gk::Vec3& color,
	       float constant_attenuation,
	       float linear_attenuation,
	       float quadratic_attenuation,
	       float specularity,
	       GLuint framebuffer = 0);

  void print() const;

  void getSceneViewProjectionTransforms(const std::vector<MyModel*> models,
					gk::Transform& view,
					gk::Transform& perspective) const;
};

#endif
