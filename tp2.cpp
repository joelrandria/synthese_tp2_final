#include "App.h"
#include "Widgets/nvSDLContext.h"

#include "ProgramManager.h"

#include "GL/GLQuery.h"
#include "GL/GLSampler.h"
#include "GL/GLTexture.h"

#include "MyFpsCamera.h"
#include "MyModel.h"
#include "MyModelFactory.h"

#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>

using namespace std;

class TP : public gk::App
{
  gk::GLCounter* m_time;
  gk::GLProgram* m_program;
  nv::SdlContext m_widgets;

  MyFpsCamera _camera;
  std::vector<MyModel*> _models;

public:

  TP()
    :gk::App(),
     _camera(gk::Point(0, 0, 50), gk::Vector(0, 1, 0), gk::Vector(0, 0, -1))
  {
    gk::AppSettings settings;
    settings.setGLVersion(3, 3);
    settings.setGLCoreProfile();
    settings.setGLDebugContext();

    if(createWindow(768, 768, settings) < 0)
      closeWindow();

    m_widgets.init();
    m_widgets.reshape(windowWidth(), windowHeight());
  }
  ~TP()
  {
  }

  int init()
  {
    gk::programPath("shaders");
    m_program = gk::createProgram("my_material.glsl");
    if(m_program == gk::GLProgram::null())
      return -1;

    loadModels();

    m_time = gk::createTimer();

    return 0;
  }

  void loadModels()
  {
    uint i;

    char filename[255];

    MyModel* model;

    const int modelSpacing = 25;
    const int modelColumnCount = 7;

    for (i = 0; i < 100; ++i)
    {
      sprintf(filename, "Bigguy/bigguy_%.2d.obj", (i % 59));

      model = MyModelFactory::createModel(filename, "bigguy_ambient.png");
      model->setPosition(gk::Point((i % modelColumnCount) * modelSpacing, 0, ((int)i / modelColumnCount) * -modelSpacing));

      _models.push_back(model);
    }

    printf("%d modèles chargés\r\n", (int)_models.size());
  }

  int quit()
  {
    return 0;
  }

  int draw()
  {
    uint i;

    MyModel* model;

    gk::Transform v;
    gk::Transform p;
    gk::Transform vp;

    gk::Transform m;
    gk::Transform mv;
    gk::Transform mvp;

    m_time->start();

    v = _camera.viewTransform();
    p = _camera.projectionTransform();
    vp = p * v;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_program->name);

    glViewport(0, 0, windowWidth(), windowHeight());

    glBindVertexArray(MyModel::sharedVertexArray());

    for (i = 0; i < _models.size(); ++i)
    {
      model = _models[i];

      m = model->modelToWorldTransform();
      mv = v * m;
      mvp = vp * m;

      m_program->uniform("mvp_matrix") = mvp.matrix();
      m_program->uniform("mv_normalmatrix") = mv.normalMatrix();

      if (model->hasDiffuseTexture())
      {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, model->diffuseTexture());
	glBindSampler(0, gk::defaultSampler()->name);

	m_program->sampler("diffuse_texture") = 0;
      }

      glDrawElementsBaseVertex(GL_TRIANGLES,
			       model->meshGpuInfo().indexCount,
			       GL_UNSIGNED_INT,
			       (GLvoid*)(sizeof(GLuint) * model->meshGpuInfo().indexOffset),
			       model->meshGpuInfo().vertexOffset);
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_time->stop();

    // UI
    {
      m_widgets.begin();
      m_widgets.beginGroup(nv::GroupFlags_GrowDownFromLeft);

      m_widgets.doLabel(nv::Rect(), m_time->summary("draw").c_str());

      m_widgets.endGroup();
      m_widgets.end();
    }

    // Clavier
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
      _camera.localTranslate(gk::Vector(0, 0, -1));
    if (key(SDLK_DOWN) || key(SDLK_s))
      _camera.localTranslate(gk::Vector(0, 0, 1));
    if (key(SDLK_LEFT) || key(SDLK_q))
      _camera.localTranslate(gk::Vector(-1, 0, 0));
    if (key(SDLK_RIGHT) || key(SDLK_d))
      _camera.localTranslate(gk::Vector(1, 0, 0));
    if (key(SDLK_PAGEUP))
      _camera.translate(gk::Vector(0, 1, 0));
    if (key(SDLK_PAGEDOWN))
      _camera.translate(gk::Vector(0, -1, 0));

    present();

    return 1;
  }

  void processWindowResize(SDL_WindowEvent& event)
  {
    m_widgets.reshape(event.data1, event.data2);
  }
  void processMouseButtonEvent(SDL_MouseButtonEvent& event)
  {
    m_widgets.processMouseButtonEvent(event);
  }
  void processMouseMotionEvent(SDL_MouseMotionEvent& event)
  {
    if (event.state & SDL_BUTTON(SDL_BUTTON_LEFT))
    {
      _camera.yaw(-event.xrel);
      _camera.pitch(-event.yrel);
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

