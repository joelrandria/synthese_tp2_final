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
			   GLuint framebuffer)
  :position(position.x, position.y, position.z, 1),
   color(color.x, color.y, color.z, 1),
   constant_attenuation(constant_attenuation),
   linear_attenuation(linear_attenuation),
   quadratic_attenuation(quadratic_attenuation),
   specularity(specularity),
   framebuffer(framebuffer)
{
}

void MyPointLight::print() const
{
  printf("Position = <%f, %f, %f>, Color = <%f, %f, %f>, Kc = %f, Kl = %f, Kq = %f\r\n",
	 (float)position.x, (float)position.y, (float)position.z,
	 (float)color.x, (float)color.y, (float)color.z,
	 (float)constant_attenuation, (float)linear_attenuation, (float)quadratic_attenuation);
}

void MyPointLight::getSceneViewProjectionTransforms(const std::vector<MyModel*> models,
						    gk::Transform& view,
						    gk::Transform& perspective) const
{
  uint i;

  gk::Point lightPosition;

  gk::BBox sceneBBox;
  gk::Point sceneBSphereCenter;
  float sceneBSphereRadius;
  float sceneDistance;

  lightPosition = gk::Point(position.x, position.y, position.z);

  for (i = 0; i < models.size(); ++i)
    sceneBBox.Union(models[i]->boundingBox());

  sceneBBox.BoundingSphere(sceneBSphereCenter, sceneBSphereRadius);

  sceneDistance = gk::Distance(lightPosition, sceneBSphereCenter);

  view = gk::LookAt(lightPosition, sceneBSphereCenter, gk::Vector(0, 1, 0));
  perspective = gk::Perspective(atan(sceneBSphereRadius / sceneDistance) * 180 / M_PI, 1, 0.01f, 1000);
}
