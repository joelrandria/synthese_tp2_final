#include "App.h"
#include "Widgets/nvSDLContext.h"

#include "ProgramManager.h"

#include "GL/GLQuery.h"
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

using namespace std;

class TP : public gk::App
{
  // gKit stuff
  gk::GLCounter* m_time;
  gk::GLProgram* m_program;
  nv::SdlContext m_widgets;

  // Caméras
  MyFpsCamera _userCamera;

  MyFpsCamera _topCamera;
  GLuint _topCameraFramebuffer;
  static const int _topCameraRenderingWidth = 256;
  static const int _topCameraRenderingHeight = 256;

  // Modèles
  std::vector<MyModel*> _models;

  // Lumières
  std::vector<MyPointLight*> _lights;

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

    for (i = 0; i < _lights.size(); ++i)
      delete _lights[i];
  }

  int init()
  {
    gk::programPath("shaders");
    m_program = gk::createProgram("my_material.glsl");
    if(m_program == gk::GLProgram::null())
      return -1;

    loadModels();

    initializeLights();
    initializeCameras();

    m_time = gk::createTimer();

    return 0;
  }

  void initializeLights()
  {
    _lights.push_back(new MyPointLight(gk::Vec4(250, 20, 0),
				       gk::Vec4(1, 1, 1),
				       100,
				       0.6f, 0, 0.0001f));
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

    glGenTextures(2, topCameraFramebufferTextures);

    glBindTexture(GL_TEXTURE_2D, topCameraFramebufferTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _topCameraRenderingWidth, _topCameraRenderingHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindTexture(GL_TEXTURE_2D, topCameraFramebufferTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _topCameraRenderingWidth, _topCameraRenderingHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &_topCameraFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _topCameraFramebuffer);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, topCameraFramebufferTextures[0], 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, topCameraFramebufferTextures[1], 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    checkFramebufferStatus("Top camera", GL_DRAW_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  void updateUserCameraProjection()
  {
    _userCamera.projectionTransform() = gk::Perspective(60, (float)windowWidth() / (float)windowHeight(), 0.01f, 1000);
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

  void loadModels()
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
      model->materialSpecularityBlending() = 0.3f;

      _models.push_back(model);
    }

    // Le sol
    model = MyModelFactory::createBox(gk::Point(-50, -14, 50), gk::Point(510, -12, -510));
    model->materialDiffuseColorEnabled() = true;
    model->materialDiffuseColor() = gk::Vec3(0.6f, 0.6f, 0.6f);
    _models.push_back(model);

    printf("%d modèle(s) chargé(s)\r\n", (int)_models.size());
  }

  int quit()
  {
    return 0;
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

    // -- 1ère passe: Vue utilisateur
    cameraRenderingPass(_userCamera, visibleModels);

    // -- 2e passe: Vue stationnaire de dessus orthographique
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _topCameraFramebuffer);
    glViewport(0, 0, _topCameraRenderingWidth, _topCameraRenderingHeight);

    cameraRenderingPass(_topCamera, visibleModels);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    // -- Composition de l'image finale
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _topCameraFramebuffer);

    glBlitFramebuffer(0, 0, _topCameraRenderingWidth, _topCameraRenderingHeight,
		      0, 0, _topCameraRenderingWidth, _topCameraRenderingHeight,
		      GL_COLOR_BUFFER_BIT,
		      GL_LINEAR);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    // -- Nettoyage des états OpenGL
    glBindVertexArray(0);

    glViewport(0, 0, windowWidth(), windowHeight());

    m_time->stop();

    // 2- Rendu UI
    {
      m_widgets.begin();
      m_widgets.beginGroup(nv::GroupFlags_GrowDownFromLeft);

      m_widgets.doLabel(nv::Rect(), m_time->summary("draw").c_str());

      m_widgets.endGroup();
      m_widgets.end();
    }

    // 3- Gestion clavier
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

    if (key(SDLK_i))
      _lights[0]->position.z--;
    if (key(SDLK_j))
      _lights[0]->position.x--;
    if (key(SDLK_l))
      _lights[0]->position.x++;
    if (key(SDLK_k))
      _lights[0]->position.z++;
    if (key(SDLK_p))
      _lights[0]->position.y++;
    if (key(SDLK_m))
      _lights[0]->position.y--;

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

    if (key('f'))
      for (uint i = 0; i < _models.size(); ++i)
	_models[i]->materialSpecularity()++;
    if (key('v'))
      for (uint i = 0; i < _models.size(); ++i)
	_models[i]->materialSpecularity()--;

    if(key(' '))
    {
      key(' ') = 0;
      _lights[0]->print();
    }

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

  void cameraRenderingPass(MyFpsCamera& camera, const std::vector<MyModel*>& models)
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
    p = camera.projectionTransform();

    vp = p * v;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_program->uniform("v_matrix") = v.matrix();

    m_program->uniform("light_position") = _lights[0]->position;
    m_program->uniform("light_color") = _lights[0]->color;
    m_program->uniform("light_constant_attenuation") = _lights[0]->constant_attenuation;
    m_program->uniform("light_linear_attenuation") = _lights[0]->linear_attenuation;
    m_program->uniform("light_quadratic_attenuation") = _lights[0]->quadratic_attenuation;

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

