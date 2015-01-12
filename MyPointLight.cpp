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
  gk::Point sceneBSphereCenterLightSpace;

  float sceneBSphereRadius;
  float sceneDistance;

  lightPosition = gk::Point(position.x, position.y, position.z);

  for (i = 0; i < models.size(); ++i)
    sceneBBox.Union(models[i]->boundingBox());

  sceneBBox.BoundingSphere(sceneBSphereCenter, sceneBSphereRadius);
  sceneDistance = gk::Distance(lightPosition, sceneBSphereCenter);

  view = gk::LookAt(lightPosition, sceneBSphereCenter, gk::Vector(0, 1, 0));

  sceneBSphereCenterLightSpace = view(sceneBSphereCenter);

  printf("Light position: "); lightPosition.print();
  printf("Scene bounding box: "); sceneBBox.print();
  printf("Scene bounding sphere: center = <%f,%f,%f>, radius = %f\r\n",
  	 sceneBSphereCenter.x, sceneBSphereCenter.y, sceneBSphereCenter.z,
  	 sceneBSphereRadius);
  printf("Scene distance: %f\r\n", sceneDistance);
  printf("FOV = %f\r\n", gk::Degrees(atan(sceneBSphereRadius / sceneDistance) * 2));

  printf("zNear = %f, zFar = %f\r\n",
	 -sceneBSphereCenterLightSpace.z - sceneBSphereRadius,
	 -sceneBSphereCenterLightSpace.z + sceneBSphereRadius);

  perspective = gk::Perspective(gk::Degrees(atan(sceneBSphereRadius / sceneDistance) * 2), 1,
				-sceneBSphereCenterLightSpace.z - sceneBSphereRadius,
				-sceneBSphereCenterLightSpace.z + sceneBSphereRadius);
}
