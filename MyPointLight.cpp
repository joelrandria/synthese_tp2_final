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
   framebuffer(0)
{
}

void MyPointLight::print() const
{
  printf("Position = <%f, %f, %f>, Color = <%f, %f, %f>, Kc = %f, Kl = %f, Kq = %f\r\n",
	 (float)position.x, (float)position.y, (float)position.z,
	 (float)color.x, (float)color.y, (float)color.z,
	 (float)constant_attenuation, (float)linear_attenuation, (float)quadratic_attenuation);
}

void MyPointLight::getBoundingPerspective(const std::vector<MyModel*> models, gk::Transform& perspective) const
{
  uint i;

  gk::BBox box;
  gk::Point sphereCenter;
  float sphereRadius;
  float distance;

  for (i = 0; i < models.size(); ++i)
    box.Union(models[i]->boundingBox());

  box.BoundingSphere(sphereCenter, sphereRadius);

  distance = gk::Distance(gk::Point(position.x, position.y, position.z), sphereCenter);

  perspective = gk::Perspective(atan(sphereRadius / distance), 1, 0.01f, 1000);
}
