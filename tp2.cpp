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

#define POINT_LIGHTS_MAX_COUNT	20

using namespace std;

class TP : public gk::App
{
  // gKit
  gk::GLCounter* m_time;
  gk::GLProgram* m_program;
  nv::SdlContext m_widgets;

  // Caméras
  MyFpsCamera _topCamera;
  MyFpsCamera _userCamera;

  // Modèles
  std::vector<MyModel*> _models;

  // Lumières
  GLuint _lightBuffer;
  std::vector<MyPointLight> _lights;

public:

  TP()
    :gk::App()
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
    gk::programPath("shaders");
    m_program = gk::createProgram("my_material.glsl");
    if(m_program == gk::GLProgram::null())
      return -1;

    initializeModels();
    initializeLights();
    initializeCameras();

    m_time = gk::createTimer();

    return 0;
  }

  void initializeModels()
  {
    uint i;

    char filename[255];

    MyModel* model;

    const int modelSpacing = 50;
    const int modelColumnCount = 10;

    // Les bigguys
    for (i = 0; i < 100; ++i)
    {
      sprintf(filename, "Bigguy/bigguy_%.2d.obj", (i % 59));

      model = MyModelFactory::createModel(filename, "bigguy_ambient.png");
      model->setPosition(gk::Point((i % modelColumnCount) * modelSpacing, 0, ((int)i / modelColumnCount) * -modelSpacing));
      model->materialSpecularity() = 100;
      model->materialSpecularityBlending() = 0.05f;

      _models.push_back(model);
    }

    // Le sol
    model = MyModelFactory::createBox(gk::Point(-50, -14, 50), gk::Point(510, -12, -510));
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
    glUniformBlockBinding(m_program->name, m_program->uniformBuffer("point_light_buffer").index, 0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    addLight();
  }
  void addLight()
  {
    char lightName[20];

    GLuint lightFramebuffer;
    GLuint lightFramebufferTextures[2];

    const int lightTextureWidth = 1024;
    const int lightTextureHeight = 1024;

    sprintf(lightName, "Light #%d", (int)_lights.size());

    glGenTextures(2, lightFramebufferTextures);

    glBindTexture(GL_TEXTURE_2D, lightFramebufferTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, lightTextureWidth, lightTextureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindTexture(GL_TEXTURE_2D, lightFramebufferTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, lightTextureWidth, lightTextureHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &lightFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, lightFramebuffer);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightFramebufferTextures[0], 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, lightFramebufferTextures[1], 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    checkFramebufferStatus(lightName, GL_DRAW_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    _lights.push_back(MyPointLight(gk::Point(250, 20, 0),
				   gk::Vec3(1, 1, 1),
				   0.6f, 0, 0.0001f,
				   100,
				   lightFramebuffer));

    commitLights();
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

    gk::Transform sceneTranslation;
    gk::Transform sceneTranslationInv;

    sceneTranslation = gk::Translate(gk::Vector(230, 0, -230));
    sceneTranslationInv = sceneTranslation.inverse();

    for (i = 0; i < _lights.size(); ++i)
    {
      p = sceneTranslationInv(gk::Point(_lights[i].position.x, _lights[i].position.y, _lights[i].position.z));
      p = gk::RotateY(1)(p);
      p = sceneTranslation(p);

      _lights[i].position = gk::glsl::vec4(p.x, p.y, p.z, 1);
    }

    commitLights();
  }
  void commitLights()
  {
    glBindBuffer(GL_UNIFORM_BUFFER, _lightBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MyPointLight) * _lights.size(), &_lights.front());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  void initializeCameras()
  {
    GLuint topCameraFramebufferTextures[2];

    _userCamera = MyFpsCamera(gk::Point(253, 25, 64), gk::Vector(0, 1, 0), gk::Vector(0, 0, -1));
    updateUserCameraProjection();

    _topCamera = MyFpsCamera(gk::Point(233, 477, -230),
			     gk::Vector(0, 0, -1),
			     gk::Vector(0, -1, 0),
			     gk::Orthographic(-300, 300, -300, 300, 0.01f, 1000));
    _topCamera.renderingWidth() = 256;
    _topCamera.renderingHeight() = 256;

    glGenTextures(2, topCameraFramebufferTextures);

    glBindTexture(GL_TEXTURE_2D, topCameraFramebufferTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _topCamera.renderingWidth(), _topCamera.renderingHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindTexture(GL_TEXTURE_2D, topCameraFramebufferTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _topCamera.renderingWidth(), _topCamera.renderingHeight(), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &_topCamera.framebuffer());
    glBindFramebuffer(GL_FRAMEBUFFER, _topCamera.framebuffer());
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, topCameraFramebufferTextures[0], 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, topCameraFramebufferTextures[1], 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    checkFramebufferStatus("Top camera", GL_DRAW_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
    updateLightsAnimation();

    return 1;
  }

  int draw()
  {
    std::vector<MyModel*> visibleModels;

    // 1- Rendu scène
    m_time->start();

    // -- Culling
    getUserVisibleModels(_models, visibleModels);

    glUseProgram(m_program->name);
    glViewport(0, 0, windowWidth(), windowHeight());
    glBindVertexArray(MyModel::sharedVertexArray());

    // -- Passe #1: Rendu profondeur de la 1ère lumière
    lightRenderingPass(_lights[0], _models);

    // -- Passe #1: Vue utilisateur
    cameraRenderingPass(_userCamera, visibleModels);

    // -- Passe #2: Vue stationnaire de dessus orthographique
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _topCamera.framebuffer());
    glViewport(0, 0, _topCamera.renderingWidth(), _topCamera.renderingHeight());

    cameraRenderingPass(_topCamera, visibleModels);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    // -- Composition de l'image finale
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _topCamera.framebuffer());

    glBlitFramebuffer(0, 0, _topCamera.renderingWidth(), _topCamera.renderingHeight(),
		      0, 0, _topCamera.renderingWidth(), _topCamera.renderingHeight(),
		      GL_COLOR_BUFFER_BIT,
		      GL_LINEAR);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

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

  void lightRenderingPass(const MyPointLight& light, const std::vector<MyModel*>& models)
  {
    gk::Transform lightPersp;

    light.getBoundingPerspective(models, lightPersp);

    
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

    v = camera.viewTransform();
    p = camera.getProjectionTransform();

    vp = p * v;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_program->uniform("v_matrix") = v.matrix();
    m_program->uniform("light_count") = (int)_lights.size();

    for (i = 0; i < models.size(); ++i)
    {
      model = models[i];
      if (!camera.isVisible(*model))
	continue;

      meshInfo = model->meshInfo();

      m = model->modelToWorldTransform();
      mv = v * m;
      mvp = vp * m;

      m_program->uniform("mv_matrix") = mv.matrix();
      m_program->uniform("mv_normalmatrix") = mv.normalMatrix();
      m_program->uniform("mvp_matrix") = mvp.matrix();

      m_program->uniform("material_diffuse_color_enabled") = model->materialDiffuseColorEnabled();
      m_program->uniform("material_diffuse_texture_enabled") = model->materialDiffuseTextureEnabled();
      m_program->uniform("material_specularity") = model->materialSpecularity();
      m_program->uniform("material_specularity_blending") = model->materialSpecularityBlending();

      if (model->materialDiffuseTextureEnabled())
      {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, model->materialDiffuseTexture());
	glBindSampler(0, gk::defaultSampler()->name);

	m_program->sampler("material_diffuse_texture") = 0;
      }
      else if (model->materialDiffuseColorEnabled())
      {
	m_program->uniform("material_diffuse_color") = model->materialDiffuseColor();
      }

      glDrawElementsBaseVertex(GL_TRIANGLES,
			       meshInfo.gpuIndexCount,
			       GL_UNSIGNED_INT,
			       (GLvoid*)(sizeof(GLuint) * meshInfo.gpuIndexOffset),
			       meshInfo.gpuVertexOffset);

      glBindTexture(GL_TEXTURE_2D, 0);
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

    // if (key(SDLK_i))
    //   _lights[0]->position.z--;
    // if (key(SDLK_j))
    //   _lights[0]->position.x--;
    // if (key(SDLK_l))
    //   _lights[0]->position.x++;
    // if (key(SDLK_k))
    //   _lights[0]->position.z++;
    // if (key(SDLK_p))
    //   _lights[0]->position.y++;
    // if (key(SDLK_m))
    //   _lights[0]->position.y--;

    // if (key('f'))
    //   _lights[0]->constant_attenuation += 0.01f;
    // if (key('v'))
    //   _lights[0]->constant_attenuation -= 0.01f;

    // if (key('g'))
    //   _lights[0]->linear_attenuation += 0.005f;
    // if (key('b'))
    //   _lights[0]->linear_attenuation -= 0.005f;
    // if (key('h'))
    //   _lights[0]->quadratic_attenuation += 0.000005f;
    // if (key('n'))
    //   _lights[0]->quadratic_attenuation -= 0.000005f;

    if (key('g'))
    {
      for (uint i = 0; i < _models.size(); ++i)
	_models[i]->materialSpecularityBlending() += 0.01f;

      printf("Bigguy's specularity blending = %f\r\n", _models[0]->materialSpecularityBlending());
    }
    if (key('b'))
    {
      for (uint i = 0; i < _models.size(); ++i)
	_models[i]->materialSpecularityBlending() -= 0.01f;

      printf("Bigguy's specularity blending = %f\r\n", _models[0]->materialSpecularityBlending());
    }

    if(key(' '))
    {
      key(' ') = 0;
      _lights[0].print();
    }
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

