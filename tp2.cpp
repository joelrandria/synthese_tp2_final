#include "App.h"
#include "Widgets/nvSDLContext.h"

#include "Mesh.h"
#include "MeshIO.h"
#include "ProgramManager.h"

#include "GL/GLBuffer.h"
#include "GL/GLBasicMesh.h"
#include "GL/GLSampler.h"
#include "GL/GLVertexArray.h"
#include "GL/GLQuery.h"

#include "MyModel.h"
#include "MyBox.h"
#include "MyFpsCamera.h"
#include "MyPointLight.h"

#include <cmath>
#include <vector>
#include <iostream>

#define WINDOW_WIDTH	1024.0f
#define WINDOW_HEIGHT	768.0f

class TpApp : public gk::App
{
private:

  // Compteurs
  gk::GLCounter* m_time;

  // Widgets & Options associés
  nv::SdlContext m_widgets;

  bool m_ambient_light_enable;
  bool m_material_reflection_enable;
  float m_material_reflection_exponent;

  // Shaders
  gk::GLProgram* m_material_program;
  gk::GLProgram* m_depthmap_program;
  gk::GLProgram* m_depthmap_rendering_program;

  // Caméra
  MyFpsCamera m_camera;
  float m_mouse_rotation_amount;

  // Modèles
  gk::GLBasicMesh* m_quad;

  std::vector<MyModel*> m_models;

  // Textures
  gk::GLSampler* m_depth_sampler;
  gk::GLSampler* m_color_sampler;

  // Lumières
  MyPointLight m_pointLight;
  bool m_pointlight_motion_enable;
  float m_pointlight_rotation_amount;

  gk::Transform m_lightViewMatrix;
  gk::Transform m_lightProjectionMatrix;

  bool m_bShadowEnable;

  GLuint m_light_depthmap;
  GLuint m_light_colormap;

  GLuint m_framebuffer;

public:

  TpApp()
    : gk::App(),

      m_ambient_light_enable(false),

      m_material_reflection_enable(false),
      m_material_reflection_exponent(15),

      m_mouse_rotation_amount(0.3f),

      m_pointLight(gk::Point(100.0f, 50.0f, 0.0f)),
      m_pointlight_motion_enable(true),
      m_pointlight_rotation_amount(5.0f),

      m_bShadowEnable(true)
    {
      // Initialisation & Congifuration du contexte OpenGL
      gk::AppSettings settings;
      settings.setGLVersion(3,3);
      settings.setGLCoreProfile();
      settings.setGLDebugContext();

      // Création de la fenêtre
      if(createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, settings) < 0)
	closeWindow();

      // Initialisation du système de widgets
      m_widgets.init();
      m_widgets.reshape(windowWidth(), windowHeight());
    }
  ~TpApp(){}

  int init()
    {
      m_time = gk::createTimer();

      loadShaders();
      loadModels();

      initializeLight();
      initializeCamera();

      return 0;
    }
  void initializeLight()
    {
      // Textures de rendu de la scène depuis la lumière
      glGenTextures(1, &m_light_colormap);
      glBindTexture(GL_TEXTURE_2D, m_light_colormap);

      glTexImage2D(GL_TEXTURE_2D, 0,
      		   GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0,
      		   GL_RGB, GL_UNSIGNED_BYTE, 0);

      glBindTexture(GL_TEXTURE_2D, 0);

      glGenTextures(1, &m_light_depthmap);
      glBindTexture(GL_TEXTURE_2D, m_light_depthmap);

      glTexImage2D(GL_TEXTURE_2D, 0,
      		   GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT, 0,
      		   GL_DEPTH_COMPONENT, GL_FLOAT, 0);

      glBindTexture(GL_TEXTURE_2D, 0);

      // Framebuffer
      glGenFramebuffers(1, &m_framebuffer);
      glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_light_colormap, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_light_depthmap, 0);

      GLenum e = glCheckFramebufferStatus(GL_FRAMEBUFFER);
      if (e != GL_FRAMEBUFFER_COMPLETE)
      {
	printf("-> FBO invalide\r\n");
	exit(-1);
      }

      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      // Samplers
      m_depth_sampler = gk::createDepthSampler();
      m_color_sampler = gk::createLinearSampler();
    }
  void initializeCamera()
    {
      m_camera = MyFpsCamera(gk::Point(-17.406164f, 31.752525f, 64.422066f),
			     gk::Vector(0.067346f, 0.978148f, -0.196702f),
			     gk::Vector(0.316839f, -0.207912f, -0.925410f),
			     gk::Perspective(60.0f, WINDOW_WIDTH / WINDOW_HEIGHT, 0.01f, 1000.0f));
    }

  void loadShaders()
    {
      gk::programPath("shaders");

      m_material_program = gk::createProgram("my_material.glsl");
      if (m_material_program == gk::GLProgram::null())
	return;

      m_depthmap_program = gk::createProgram("my_depthmap.glsl");
      if (m_depthmap_program == gk::GLProgram::null())
	return;

      m_depthmap_rendering_program = gk::createProgram("my_depthmap_rendering.glsl");
      if (m_depthmap_rendering_program == gk::GLProgram::null())
	return;
    }
  void loadModels()
    {
      // Mesh internes
      m_quad = new gk::GLBasicMesh(GL_TRIANGLE_STRIP, 4);

      // Modèles externes
      m_models.push_back(new MyModel("Bigguy/bigguy_00.obj",
				     gk::Translate(gk::Vector(20, 9.5f, 20))));
      m_models.push_back(new MyModel("Bigguy/bigguy_01.obj",
				     gk::Translate(gk::Vector(-20, 9.5f, 20))));
      m_models.push_back(new MyModel("Bigguy/bigguy_02.obj",
				     gk::Translate(gk::Vector(20, 9.5f, -20))));
      m_models.push_back(new MyModel("Bigguy/bigguy_03.obj",
				     gk::Translate(gk::Vector(-20, 9.5f, -20))));

      for (unsigned int i = 0; i < m_models.size(); ++i)
      {
	m_models[i]->setMaterialDiffuseColor(gk::VecColor(1, 1, 1));
	m_models[i]->enableMaterialReflection(true);
	m_models[i]->setMaterialReflectionExponent(20);

	gk::VecColor specularColor;
	if (i == 0)
	  specularColor = gk::VecColor(1, 0, 0);
	else if (i == 1)
	  specularColor = gk::VecColor(0, 1, 0);
	else if (i == 2)
	  specularColor = gk::VecColor(0, 0, 1);

	m_models[i]->setMaterialReflectionColor(specularColor);
      }

      MyBox* ground = new MyBox(gk::Scale(100, 1, 100));
      ground->setMaterialDiffuseColor(gk::VecColor(1, 1, 1));
      ground->enableMaterialReflection(true);
      ground->setMaterialReflectionExponent(2);

      m_models.push_back(ground);
    }

  void shadowMapRenderingPass()
    {
      uint i;

      glUseProgram(m_depthmap_program->name);

      // Bounding sphere de la scène
      gk::BBox bBox;
      float bSphereRadius;
      gk::Point bSphereCenter;

      float lightTanViewAngle;
      gk::Transform lightProjectionViewMatrix;
      gk::Point bSphereCenterLightSpace;

      for (i = 0; i < m_models.size(); ++i)
      	if (m_camera.isVisible(*m_models[i]))
      	  bBox.Union(m_models[i]->worldBBox());

      bBox.BoundingSphere(bSphereCenter, bSphereRadius);

      // Matrice ViewProjection d'observation de la scène depuis la lumière
      m_lightViewMatrix = gk::LookAt(m_pointLight.position(),
				     bSphereCenter,
				     gk::Vector(0, 1, 0));

      bSphereCenterLightSpace = m_lightViewMatrix(bSphereCenter);
      lightTanViewAngle = gk::Distance(m_pointLight.position(), bSphereCenter) / bSphereRadius;

      m_lightProjectionMatrix = gk::Perspective(gk::Degrees(lightTanViewAngle),
						WINDOW_WIDTH / WINDOW_HEIGHT,
						-bSphereCenterLightSpace.z - bSphereRadius,
						-bSphereCenterLightSpace.z + bSphereRadius);

      lightProjectionViewMatrix = m_lightProjectionMatrix * m_lightViewMatrix;

      // Rendu framebuffer du point de vue de la lumière
      glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glDrawBuffer(GL_COLOR_ATTACHMENT0);

      for (i = 0; i < m_models.size(); ++i)
      {
      	gk::Transform lightMvpMatrix = lightProjectionViewMatrix * m_models[i]->transform();

      	m_depthmap_program->uniform("light_mvp_matrix") = lightMvpMatrix.matrix();

      	glBindVertexArray(m_models[i]->name());

      	glDrawElements(GL_TRIANGLES, m_models[i]->indicesSize(), GL_UNSIGNED_INT, 0);
      }

      // Réinitialisation de la pipeline
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glBindVertexArray(0);
      glUseProgram(0);
    }
  void sceneRenderingPass()
    {
      uint i;

      glUseProgram(m_material_program->name);

      // Assignation des variables des shaders
      m_material_program->uniform("view_position") = m_camera.position();

      m_material_program->uniform("light_position") = m_pointLight.position();
      m_material_program->uniform("light_color") = gk::VecColor(1, 1, 1);

      m_material_program->uniform("ambient_light_enable") = m_ambient_light_enable;
      m_material_program->uniform("ambient_light_color") = gk::VecColor(0.1f, 0.1f, 0.1f);

      for (i = 0; i < m_models.size(); ++i)
      {
      	if (m_camera.isVisible(*m_models[i]))
      	{
	  gk::Transform mvpMat = m_camera.projection() * m_camera.view() * m_models[i]->transform();

	  // Attributs caméra
	  m_material_program->uniform("model_matrix") = m_models[i]->transform().matrix();
	  m_material_program->uniform("normal_matrix") = m_models[i]->transform().normalMatrix();
      	  m_material_program->uniform("mvp_matrix") = mvpMat.matrix();

	  // Attributs matériau
	  m_material_program->uniform("material_diffuse_color") = m_models[i]->getMaterialDiffuseColor();

	  m_material_program->uniform("material_reflection_enable") = m_models[i]->isMaterialReflectionEnable();
	  m_material_program->uniform("material_reflection_exponent") = m_models[i]->getMaterialReflectionExponent();
	  m_material_program->uniform("material_reflection_color") = m_models[i]->getMaterialReflectionColor();

	  // Attributs shadow mapping
	  m_material_program->uniform("shadow_enable") = m_bShadowEnable;

	  m_material_program->uniform("light_view_matrix") = m_lightViewMatrix.matrix();
	  m_material_program->uniform("light_projection_matrix") = m_lightProjectionMatrix.matrix();
	  m_material_program->uniform("light_viewport_matrix") = gk::Viewport(WINDOW_WIDTH, WINDOW_HEIGHT).matrix();

	  m_material_program->uniform("light_depth_texture_width") = WINDOW_WIDTH;
	  m_material_program->uniform("light_depth_texture_height") = WINDOW_HEIGHT;

	  m_material_program->sampler("light_depth_texture") = 0;
	  glActiveTexture(GL_TEXTURE0);
	  glBindTexture(GL_TEXTURE_2D, m_light_depthmap);
	  glGenerateMipmap(GL_TEXTURE_2D);
	  glBindSampler(0, m_depth_sampler->name);

      	  glBindVertexArray(m_models[i]->name());

      	  glDrawElements(GL_TRIANGLES, m_models[i]->indicesSize(), GL_UNSIGNED_INT, 0);
	}
      }

      // Réinitialisation des informations
      glUseProgram(0);
      glBindVertexArray(0);
    }
  void shadowMapViewRenderingPass()
    {
      glScissor(0, 0, WINDOW_WIDTH / 3, WINDOW_HEIGHT / 3);
      glEnable(GL_SCISSOR_TEST);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glDisable(GL_SCISSOR_TEST);
      glViewport(0, 0, WINDOW_WIDTH / 3, WINDOW_HEIGHT / 3);

      glUseProgram(m_depthmap_rendering_program->name);

      m_depthmap_rendering_program->sampler("color_texture") = 0;
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, m_light_colormap);
      glGenerateMipmap(GL_TEXTURE_2D);
      glBindSampler(0, m_color_sampler->name);

      m_depthmap_rendering_program->sampler("depth_texture") = 1;
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, m_light_depthmap);
      glGenerateMipmap(GL_TEXTURE_2D);
      glBindSampler(1, m_depth_sampler->name);

      m_quad->draw();

      glUseProgram(0);
    }

  int draw()
    {
      glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Scene rendering passes
      m_time->start();

      shadowMapRenderingPass();
      sceneRenderingPass();
      shadowMapViewRenderingPass();

      m_time->stop();

      // UI
      glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

      m_widgets.begin();
      m_widgets.beginGroup(nv::GroupFlags_GrowDownFromLeft);

      m_widgets.doLabel(nv::Rect(), m_time->summary("draw").c_str());

      m_widgets.doButton(nv::Rect(), "Eclairage ambient", &m_ambient_light_enable);
      m_widgets.doButton(nv::Rect(), "Reflexions", &m_material_reflection_enable);
      m_widgets.doButton(nv::Rect(), "Ombres", &m_bShadowEnable);

      if (m_material_reflection_enable)
      {
	char str[128];

	sprintf(str, "Exposant: %d", (int)m_material_reflection_exponent);
	m_widgets.doLabel(nv::Rect(0, 0, 1000, 0), str);
	m_widgets.doHorizontalSlider(nv::Rect(0, 0, 1000, 0), 1.f, 1000.f, &m_material_reflection_exponent);
      }

      m_widgets.endGroup();
      m_widgets.end();

      // MàJ des propriétés de matériau des bigguys
      updateBigguyMaterialProperties();

      // Présentation du rendu
      present();

      return 1;
    }

  virtual int update(const int time, const int delta)
    {
      // Gestion des évènements claviers
      if (key('z'))
	m_camera.localTranslate(gk::Vector(0, 0, -1));
      if (key('q'))
	m_camera.localTranslate(gk::Vector(-1, 0, 0));
      if (key('d'))
	m_camera.localTranslate(gk::Vector(1, 0, 0));
      if (key('s'))
	m_camera.localTranslate(gk::Vector(0, 0, 1));

      if (key(' '))
	m_pointlight_motion_enable = !m_pointlight_motion_enable;
      if (key('p'))
	m_pointlight_rotation_amount += 1.0f;
      if (key('m'))
	m_pointlight_rotation_amount -= 1.0f;

      // Animation de la lumière
      if (m_pointlight_motion_enable)
      {
	float pointlight_rotation_angle = (m_pointlight_rotation_amount * (float)delta) / 1000.0f;
	gk::Transform pointlight_rotation = gk::Rotate(pointlight_rotation_angle, gk::Vector(0, 1, 0));
	m_pointLight.position() = pointlight_rotation(m_pointLight.position());
      }

      return gk::App::update(time, delta);
    }

  void processKeyboardEvent( SDL_KeyboardEvent& event )
    {
      m_widgets.processKeyboardEvent(event);
    }
  void processWindowResize( SDL_WindowEvent& event )
    {
      m_widgets.reshape(event.data1, event.data2);
    }
  void processMouseButtonEvent( SDL_MouseButtonEvent& event )
    {
      m_widgets.processMouseButtonEvent(event);
    }
  void processMouseMotionEvent(SDL_MouseMotionEvent& event)
    {
      if (key(SDLK_LCTRL))
      {
	m_camera.yaw(m_mouse_rotation_amount * -event.xrel);
	m_camera.pitch(m_mouse_rotation_amount * -event.yrel);
      }

      m_widgets.processMouseMotionEvent(event);
    }

private:

  void updateBigguyMaterialProperties()
    {
      for (int i = 0; i < 4; ++i)
      {
	m_models[i]->enableMaterialReflection(m_material_reflection_enable);
	m_models[i]->setMaterialReflectionExponent(m_material_reflection_exponent);
      }
    }

};

int main(int, char**)
{
  TpApp app;
  app.run();

  return 0;
}
