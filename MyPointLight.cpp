#include "MyPointLight.h"
#include "MyModel.h"

#include <math.h>
#include <stdio.h>

MyPointLight::MyPointLight(const gk::Point& position,
			   const gk::Vec3& color,
			   float constant_attenuation,
			   float linear_attenuation,
			   float quadratic_attenuation,
			   float specularity,
			   GLuint shadow_framebuffer,
			   GLuint shadow_depth_texture)
  :position(position.x, position.y, position.z, 1),
   color(color.x, color.y, color.z, 1),
   constant_attenuation(constant_attenuation),
   linear_attenuation(linear_attenuation),
   quadratic_attenuation(quadratic_attenuation),
   specularity(specularity),
   shadow_framebuffer(shadow_framebuffer),
  shadow_depth_texture(shadow_depth_texture)
{
}

void MyPointLight::print() const
{
  printf("Position = <%f, %f, %f>, Color = <%f, %f, %f>, Kc = %f, Kl = %f, Kq = %f\r\n",
	 (float)position.x, (float)position.y, (float)position.z,
	 (float)color.x, (float)color.y, (float)color.z,
	 (float)constant_attenuation, (float)linear_attenuation, (float)quadratic_attenuation);
}

void MyPointLight::updateShadowMapMatrices(const std::vector<MyModel*> models)
{
  uint i;

  gk::Point lightPosition;

  gk::BBox sceneBBox;
  gk::Point sceneBSphereCenter;
  gk::Point sceneBSphereCenterLightSpace;

  float sceneBSphereRadius;
  float sceneDistance;

  float fovDegrees;

  gk::Transform view;
  gk::Transform perspective;

  lightPosition = gk::Point(position.x, position.y, position.z);

  for (i = 0; i < models.size(); ++i)
    sceneBBox.Union(models[i]->boundingBox());

  sceneBBox.BoundingSphere(sceneBSphereCenter, sceneBSphereRadius);
  sceneDistance = gk::Distance(lightPosition, sceneBSphereCenter);

  view = gk::LookAt(lightPosition, sceneBSphereCenter, gk::Vector(0, 1, 0));

  sceneBSphereCenterLightSpace = view(sceneBSphereCenter);

  fovDegrees = gk::Degrees(asin(sceneBSphereRadius / sceneDistance)) * 2;

  // printf("Light position: "); lightPosition.print();
  // printf("Scene bounding box: "); sceneBBox.print();
  // printf("Scene bounding sphere: center = <%f,%f,%f>, radius = %f\r\n",
  // 	 sceneBSphereCenter.x, sceneBSphereCenter.y, sceneBSphereCenter.z,
  // 	 sceneBSphereRadius);
  // printf("Scene distance: %f\r\n", sceneDistance);
  // printf("FOV = %f\r\n", fovDegrees);

  // printf("zNear = %f, zFar = %f\r\n",
  // 	 -sceneBSphereCenterLightSpace.z - sceneBSphereRadius,
  // 	 -sceneBSphereCenterLightSpace.z + sceneBSphereRadius);

  perspective = gk::Perspective(fovDegrees,
				1,
				-sceneBSphereCenterLightSpace.z - sceneBSphereRadius,
				-sceneBSphereCenterLightSpace.z + sceneBSphereRadius);

  shadowmap_vp_matrix = (perspective * view).matrix();
  shadowmap_vpi_matrix = (gk::Viewport(1, 1) * perspective * view).matrix();
}
