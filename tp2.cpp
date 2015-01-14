#include "App.h"
#include "Widgets/nvSDLContext.h"

#include "ProgramManager.h"

#include "GL/GLQuery.h"
#include "GL/GLBuffer.h"
#include "GL/GLSampler.h"
#include "GL/GLTexture.h"

#include "MyFpsCamera.h"
#include "MyModel.h"
#include "MyModelFactory.h"
#include "MyMeshInfo.h"
#include "MyPointLight.h"

#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>

#define POINT_LIGHTS_MAX_COUNT		5
#define POINT_LIGHT_FB_TEXTURE_WIDTH	4096
#define POINT_LIGHT_FB_TEXTURE_HEIGHT	4096

using namespace std;

class TP : public gk::App
{
  // gKit
  gk::GLCounter* m_time;
  nv::SdlContext m_widgets;

  gk::GLProgram* m_basicProgram;
  gk::GLProgram* m_renderingProgram;

  // Caméras
  MyFpsCamera _userCamera;

  // Modèles
  std::vector<MyModel*> _models;

  // Lumières
  GLuint _lightBuffer;
  bool _lightAnimationEnabled;
  std::vector<MyPointLight> _lights;

public:

  TP()
    :gk::App(),
     _lightAnimationEnabled(false)
  {
    gk::AppSettings settings;
    settings.setGLVersion(3, 3);
    settings.setGLCoreProfile();
    settings.setGLDebugContext();

    if(createWindow(1400, 1000, settings) < 0)
      closeWindow();

    m_widgets.init();
    m_widgets.reshape(windowWidth(), windowHeight());
  }
  ~TP()
  {
    uint i;

    for (i = 0; i < _models.size(); ++i)
      delete _models[i];
  }

  int init()
  {
    if (loadPrograms() < 0)
      return -1;

    loadModels();

    initializeLights();
    initializeCameras();

    m_time = gk::createTimer();

    return 0;
  }

  int loadPrograms()
  {
    m_basicProgram = gk::createProgram("my_basic_program.glsl");
    if (m_basicProgram == gk::GLProgram::null())
      return -1;

    m_renderingProgram = gk::createProgram("my_rendering_program.glsl");
    if (m_renderingProgram == gk::GLProgram::null())
      return -1;

    return 0;
  }

  void loadModels()
  {
    int i;
    int r;
    int c;

    char filename[255];

    MyModel* model;
    const int modelSpacing = 30;

    // Les bigguys
    i = 0;

    for (r = 0; r < 3; ++r)
    {
      for (c = 0; c < 3; ++c)
      {
	sprintf(filename, "Bigguy/bigguy_%.2d.obj", (i++ % 59));

	model = MyModelFactory::createModel(filename, "bigguy_ambient.png");
	model->setPosition(gk::Point((r - 1) * modelSpacing, 0, (c - 1) * -modelSpacing));
	model->materialSpecularity() = 100;
	model->materialSpecularityBlending() = 0.05f;
	_models.push_back(model);
      }
    }

    // Le sol
    model = MyModelFactory::createBox(gk::Point(-75, -14, 75), gk::Point(75, -9, -75));
    model->materialDiffuseColorEnabled() = true;
    model->materialDiffuseColor() = gk::Vec3(0.6f, 0.6f, 0.6f);
    _models.push_back(model);
  }

  void initializeLights()
  {
    glGenBuffers(1, &_lightBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, _lightBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(MyPointLight) * POINT_LIGHTS_MAX_COUNT, 0, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, _lightBuffer);
    glUniformBlockBinding(m_renderingProgram->name, m_renderingProgram->uniformBuffer("point_light_buffer").index, 0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    addLight();
  }
  void addLight()
  {
    char lightName[20];

    GLuint lightFramebuffer;
    GLuint lightFramebufferTextures[2];

    sprintf(lightName, "Light #%d", (int)_lights.size());

    glGenTextures(2, lightFramebufferTextures);

    glBindTexture(GL_TEXTURE_2D, lightFramebufferTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, POINT_LIGHT_FB_TEXTURE_WIDTH, POINT_LIGHT_FB_TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    glBindTexture(GL_TEXTURE_2D, lightFramebufferTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, POINT_LIGHT_FB_TEXTURE_WIDTH, POINT_LIGHT_FB_TEXTURE_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
    glGenFramebuffers(1, &lightFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, lightFramebuffer);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightFramebufferTextures[0], 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, lightFramebufferTextures[1], 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    checkFramebufferStatus(lightName, GL_FRAMEBUFFER);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    _lights.push_back(MyPointLight(gk::Point(0, 75, 175),
				   gk::Vec3(1, 1, 1),
				   0.6f, 0, 0.000025f,
				   100,
				   lightFramebuffer,
				   lightFramebufferTextures[1]));

    updateLightsAnimation();
  }
  void removeLight()
  {
    _lights.pop_back();

    commitLights();
  }
  void updateLightsAnimation()
  {
    uint i;
    gk::Point p;

    for (i = 0; i < _lights.size(); ++i)
    {
      p = gk::RotateY(1)(gk::Point(_lights[i].position.x, _lights[i].position.y, _lights[i].position.z));

      _lights[i].position = gk::glsl::vec4(p.x, p.y, p.z, 1);
    }

    commitLights();
  }
  void commitLights()
  {
    uint i;
    for (i = 0; i < _lights.size(); ++i)
      _lights[i].updateShadowMapMatrices(_models);

    glBindBuffer(GL_UNIFORM_BUFFER, _lightBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MyPointLight) * _lights.size(), &_lights.front());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  void initializeCameras()
  {
    _userCamera = MyFpsCamera(gk::Point(0, 25, 140), gk::Vector(0, 1, 0), gk::Vector(0, 0, -1));

    updateUserCameraProjection();
  }
  void updateUserCameraProjection()
  {
    _userCamera.setProjectionTransform(gk::Perspective(60, (float)windowWidth() / (float)windowHeight(), 0.01f, 1000));
  }

  void checkFramebufferStatus(const std::string& name, GLenum target)
  {
    switch (glCheckFramebufferStatus(target))
    {
    case GL_FRAMEBUFFER_UNDEFINED: fprintf(stderr, "Framebuffer '%s' incomplet: GL_FRAMEBUFFER_UNDEFINED\r\n", name.c_str()); exit(-1);
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: fprintf(stderr, "Framebuffer '%s' incomplet: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\r\n", name.c_str()); exit(-1);
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: fprintf(stderr, "Framebuffer '%s' incomplet: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\r\n", name.c_str()); exit(-1);
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: fprintf(stderr, "Framebuffer '%s' incomplet: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\r\n", name.c_str()); exit(-1);
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: fprintf(stderr, "Framebuffer '%s' incomplet: GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER\r\n", name.c_str()); exit(-1);
    case GL_FRAMEBUFFER_UNSUPPORTED: fprintf(stderr, "Framebuffer '%s' incomplet: GL_FRAMEBUFFER_UNSUPPORTED\r\n", name.c_str()); exit(-1);
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: fprintf(stderr, "Framebuffer '%s' incomplet: GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE\r\n", name.c_str()); exit(-1);
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: fprintf(stderr, "Framebuffer '%s' incomplet: GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS\r\n", name.c_str()); exit(-1);

    default: printf("Framebuffer '%s' OK\r\n", name.c_str());
    }
  }

  int quit()
  {
    return 0;
  }

  int update(const int time, const int delta)
  {
    if (_lightAnimationEnabled)
      updateLightsAnimation();

    return 1;
  }

  int draw()
  {
    std::vector<MyModel*> visibleModels;

    // 1- Rendu scène
    m_time->start();

    glBindVertexArray(MyModel::sharedVertexArray());

    // -- Culling
    getUserVisibleModels(_models, visibleModels);

    // -- Passe #1: Cartes de profondeurs des sources de lumières
    shadowMapRenderingPass(_models);

    // -- Passe #2: Vue utilisateur
    cameraRenderingPass(_userCamera, visibleModels);

    // -- Viewport depth buffers
    depthBuffersRenderingPass();

    // -- Nettoyage des états OpenGL
    glBindVertexArray(0);

    m_time->stop();

    ui();

    present();

    return 1;
  }

  void getUserVisibleModels(const std::vector<MyModel*>& models, std::vector<MyModel*>& visible)
  {
    uint i;

    visible.clear();

    for (i = 0; i < models.size(); ++i)
      if (_userCamera.isVisible(*models[i]))
	visible.push_back(models[i]);
  }

  void shadowMapRenderingPass(const std::vector<MyModel*>& models)
  {
    uint i;
    uint j;

    MyModel* model;
    MyMeshInfo meshInfo;

    gk::Transform vp;
    gk::Transform mvp;

    glUseProgram(m_basicProgram->name);

    for (i = 0; i < _lights.size(); ++i)
    {
      vp = gk::Transform(_lights[i].shadowmap_vp_matrix);

      glBindFramebuffer(GL_FRAMEBUFFER, _lights[i].shadow_framebuffer);
      glViewport(0, 0, POINT_LIGHT_FB_TEXTURE_WIDTH, POINT_LIGHT_FB_TEXTURE_HEIGHT);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      for (j = 0; j < models.size(); ++j)
      {
	model = models[j];

	meshInfo = model->meshInfo();

	mvp = vp * model->modelToWorldTransform();

	m_basicProgram->uniform("mvp_matrix") = mvp.matrix();

	glDrawElementsBaseVertex(GL_TRIANGLES,
				 meshInfo.gpuIndexCount,
				 GL_UNSIGNED_INT,
				 (GLvoid*)(sizeof(GLuint) * meshInfo.gpuIndexOffset),
				 meshInfo.gpuVertexOffset);
      }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  void cameraRenderingPass(const MyFpsCamera& camera, const std::vector<MyModel*>& models)
  {
    uint i;

    MyModel* model;
    MyMeshInfo meshInfo;

    gk::Transform v;
    gk::Transform p;
    gk::Transform vp;

    gk::Transform m;
    gk::Transform mv;
    gk::Transform mvp;

    char samplerName[50];
    static gk::GLSampler* depthSampler = gk::createLinearSampler();

    v = camera.viewTransform();
    p = camera.getProjectionTransform();

    vp = p * v;

    glUseProgram(m_renderingProgram->name);

    glViewport(0, 0, windowWidth(), windowHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_renderingProgram->uniform("v_matrix") = v.matrix();
    m_renderingProgram->uniform("light_count") = (int)_lights.size();

    for (i = 0; i < _lights.size(); ++i)
    {
      if (i >= 5)
	break;

      sprintf(samplerName, "light_shadow_depth_texture%d", (int)i);

      glActiveTexture(GL_TEXTURE1 + i);
      glBindTexture(GL_TEXTURE_2D, _lights[i].shadow_depth_texture);
      glBindSampler(1 + i, depthSampler->name);
      glGenerateMipmap(GL_TEXTURE_2D);

      m_renderingProgram->sampler(samplerName) = 1 + (int)i;
    }

    for (i = 0; i < models.size(); ++i)
    {
      model = models[i];
      if (!camera.isVisible(*model))
	continue;

      meshInfo = model->meshInfo();

      m = model->modelToWorldTransform();
      mv = v * m;
      mvp = vp * m;

      m_renderingProgram->uniform("m_matrix") = m.matrix();
      m_renderingProgram->uniform("m_normalmatrix") = m.normalMatrix();
      m_renderingProgram->uniform("mv_matrix") = mv.matrix();
      m_renderingProgram->uniform("mv_normalmatrix") = mv.normalMatrix();
      m_renderingProgram->uniform("mvp_matrix") = mvp.matrix();

      m_renderingProgram->uniform("material_diffuse_color_enabled") = model->materialDiffuseColorEnabled();
      m_renderingProgram->uniform("material_diffuse_texture_enabled") = model->materialDiffuseTextureEnabled();
      m_renderingProgram->uniform("material_specularity") = model->materialSpecularity();
      m_renderingProgram->uniform("material_specularity_blending") = model->materialSpecularityBlending();

      if (model->materialDiffuseTextureEnabled())
      {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, model->materialDiffuseTexture());
	glBindSampler(0, gk::defaultSampler()->name);

	m_renderingProgram->sampler("material_diffuse_texture") = 0;
      }
      else if (model->materialDiffuseColorEnabled())
      {
	m_renderingProgram->uniform("material_diffuse_color") = model->materialDiffuseColor();
      }

      glDrawElementsBaseVertex(GL_TRIANGLES,
			       meshInfo.gpuIndexCount,
			       GL_UNSIGNED_INT,
			       (GLvoid*)(sizeof(GLuint) * meshInfo.gpuIndexOffset),
			       meshInfo.gpuVertexOffset);

      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }

  void depthBuffersRenderingPass()
  {
    uint i;
    const int viewportSize = 256;

    for (i = 0; i < _lights.size(); ++i)
    {
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
      glBindFramebuffer(GL_READ_FRAMEBUFFER, _lights[i].shadow_framebuffer);

      glBlitFramebuffer(0, 0, POINT_LIGHT_FB_TEXTURE_WIDTH, POINT_LIGHT_FB_TEXTURE_HEIGHT,
			viewportSize * i, 0, viewportSize * (i + 1), 256,
			GL_COLOR_BUFFER_BIT,
			GL_LINEAR);
    }
  }

  void ui()
  {
    glViewport(0, 0, windowWidth(), windowHeight());

    // Widgets
    {
      m_widgets.begin();
      m_widgets.beginGroup(nv::GroupFlags_GrowDownFromLeft);

      m_widgets.doLabel(nv::Rect(), m_time->summary("draw").c_str());

      m_widgets.endGroup();
      m_widgets.end();
    }

    // Gestion clavier
    if (key(SDLK_ESCAPE))
      closeWindow();

    if (key('r'))
    {
      key('r') = 0;
      gk::reloadPrograms();
    }
    if(key('c'))
    {
      key('c') = 0;
      gk::writeFramebuffer("screenshot.png");
    }

    // Contrôles clavier caméra
    if (key(SDLK_UP) || key(SDLK_z))
      _userCamera.localTranslate(gk::Vector(0, 0, -1));
    if (key(SDLK_DOWN) || key(SDLK_s))
      _userCamera.localTranslate(gk::Vector(0, 0, 1));
    if (key(SDLK_LEFT) || key(SDLK_q))
      _userCamera.localTranslate(gk::Vector(-1, 0, 0));
    if (key(SDLK_RIGHT) || key(SDLK_d))
      _userCamera.localTranslate(gk::Vector(1, 0, 0));
    if (key(SDLK_PAGEUP))
      _userCamera.localTranslate(gk::Vector(0, 1, 0));
    if (key(SDLK_PAGEDOWN))
      _userCamera.localTranslate(gk::Vector(0, -1, 0));

    // Contrôles clavier lumières
    if (key(SDLK_KP_PLUS))
    {
      key(SDLK_KP_PLUS) = 0;
      addLight();
    }
    if (key(SDLK_KP_MINUS))
    {
      key(SDLK_KP_MINUS) = 0;
      removeLight();
    }

    if (key(' '))
    {
      key(' ') = 0;
      _lightAnimationEnabled = !_lightAnimationEnabled;
    }

    if (key(SDLK_KP_8))
      _lights[0].position.z = _lights[0].position.z - 1;
    if (key(SDLK_KP_5))
      _lights[0].position.z = _lights[0].position.z + 1;
    if (key(SDLK_KP_6))
      _lights[0].position.x = _lights[0].position.x + 1;
    if (key(SDLK_KP_4))
      _lights[0].position.x = _lights[0].position.x - 1;
    if (key(SDLK_KP_7))
      _lights[0].position.y = _lights[0].position.y + 1;
    if (key(SDLK_KP_9))
      _lights[0].position.y = _lights[0].position.y - 1;
    if (key(SDLK_KP_0))
      _lights[0].print();

    if (key('f'))
      _lights[0].constant_attenuation += 0.01f;
    if (key('v'))
      _lights[0].constant_attenuation -= 0.01f;
    if (key('g'))
      _lights[0].linear_attenuation += 0.01f;
    if (key('b'))
      _lights[0].linear_attenuation -= 0.01f;
    if (key('h'))
      _lights[0].quadratic_attenuation += 0.01f;
    if (key('n'))
      _lights[0].quadratic_attenuation -= 0.01f;

    commitLights();
  }

  void processWindowResize(SDL_WindowEvent& event)
  {
    m_widgets.reshape(event.data1, event.data2);

    updateUserCameraProjection();
  }
  void processMouseButtonEvent(SDL_MouseButtonEvent& event)
  {
    m_widgets.processMouseButtonEvent(event);
  }
  void processMouseMotionEvent(SDL_MouseMotionEvent& event)
  {
    if (event.state & SDL_BUTTON(SDL_BUTTON_LEFT))
    {
      _userCamera.yaw(-event.xrel);
      _userCamera.pitch(-event.yrel);
    }

    m_widgets.processMouseMotionEvent(event);
  }
  void processKeyboardEvent(SDL_KeyboardEvent& event)
  {
    m_widgets.processKeyboardEvent(event);
  }

};


int main( int argc, char **argv )
{
  TP app;
  app.run();

  return 0;
}

