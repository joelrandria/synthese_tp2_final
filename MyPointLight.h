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

  gk::Matrix4x4 shadowmap_vp_matrix;
  gk::Matrix4x4 shadowmap_vpi_rmatrix;

  GLuint shadow_framebuffer;
  GLuint shadow_depth_texture;

  MyPointLight(const gk::Point& position,
	       const gk::Vec3& color,
	       float constant_attenuation,
	       float linear_attenuation,
	       float quadratic_attenuation,
	       float specularity,
	       GLuint shadow_framebuffer = 0,
	       GLuint shadow_depth_texture = 0);

  void print() const;

  void updateShadowMapMatrices(const std::vector<MyModel*> models);
};

#endif
