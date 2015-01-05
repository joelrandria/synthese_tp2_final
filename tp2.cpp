#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>

#include "App.h"
#include "Widgets/nvSDLContext.h"

#include "Mesh.h"
#include "MeshIO.h"

#include "GL/GLQuery.h"
#include "GL/GLTexture.h"
#include "GL/GLBuffer.h"
#include "GL/GLVertexArray.h"

#include "ProgramManager.h"

#include "MyModel.h"
#include "MyFpsCamera.h"

using namespace std;

struct Format
{
  char text[1024];

  Format( const char *_format, ... )
  {
    text[0] = 0;

    va_list args;
    va_start(args, _format);
    vsnprintf(text, sizeof(text), _format, args);
    va_end(args);
  }
  ~Format( ) {}

  operator const char *( )
  {
    return text;
  }
};

class TP : public gk::App
{
  nv::SdlContext m_widgets;

  gk::GLProgram *m_program;

  int m_indices_size;
  gk::GLVertexArray *m_vao;

  gk::GLCounter *m_time;

  MyFpsCamera _camera;

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
  ~TP( ) {}

  int init( )
  {
    gk::programPath("shaders");
    m_program = gk::createProgram("dFnormal.glsl");
    if(m_program == gk::GLProgram::null())
      return -1;

    gk::Mesh* mesh = gk::MeshIO::readOBJ("Bigguy/bigguy_00.obj");
    if(mesh == NULL)
      return -1;

    m_vao = gk::createVertexArray();

    gk::GLBuffer* vertexPositionBuffer = gk::createBuffer(GL_ARRAY_BUFFER, mesh->positions);
    glVertexAttribPointer(m_program->attribute("position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(m_program->attribute("position"));

    gk::GLBuffer* indexBuffer = gk::createBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indices);
    m_indices_size = mesh->indices.size();

    delete mesh;
    delete vertexPositionBuffer;
    delete indexBuffer;

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m_time = gk::createTimer();

    return 0;
  }

  int quit( )
  {
    return 0;
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
      _camera.Yaw(-event.xrel);
      _camera.Pitch(-event.yrel);
    }

    m_widgets.processMouseMotionEvent(event);
  }
  void processKeyboardEvent(SDL_KeyboardEvent& event)
  {
    m_widgets.processKeyboardEvent(event);
  }

  int draw()
  {
    if(key(SDLK_ESCAPE))
      // fermer l'application si l'utilisateur appuie sur ESCAPE
      closeWindow();

    if(key('r'))
    {
      key('r')= 0;
      // recharge et recompile les shaders
      gk::reloadPrograms();
    }
    if(key('c'))
    {
      key('c')= 0;
      // enregistre l'image opengl
      gk::writeFramebuffer("screenshot.png");
    }

    if(key(SDLK_UP) || key(SDLK_z))
      _camera.LocalTranslate(gk::Vector(0, 0, -1));
    if(key(SDLK_DOWN) || key(SDLK_s))
      _camera.LocalTranslate(gk::Vector(0, 0, 1));
    if(key(SDLK_LEFT) || key(SDLK_q))
      _camera.LocalTranslate(gk::Vector(-1, 0, 0));
    if(key(SDLK_RIGHT) || key(SDLK_d))
      _camera.LocalTranslate(gk::Vector(1, 0, 0));

    glViewport(0, 0, windowWidth(), windowHeight());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_time->start();

    gk::Transform model;
    gk::Transform vp = _camera.ViewProjectionTransform();
    gk::Transform mvp = vp * model;

    glUseProgram(m_program->name);

    m_program->uniform("mvpMatrix") = mvp.matrix();
    m_program->uniform("diffuse_color") = gk::VecColor(1, 1, 0);

    glBindVertexArray(m_vao->name);
    glDrawElements(GL_TRIANGLES, m_indices_size, GL_UNSIGNED_INT, 0);

    glUseProgram(0);
    glBindVertexArray(0);

    m_time->stop();

    // UI
    {
      m_widgets.begin();
      m_widgets.beginGroup(nv::GroupFlags_GrowDownFromLeft);

      m_widgets.doLabel(nv::Rect(), m_time->summary("draw").c_str());

      m_widgets.endGroup();
      m_widgets.end();
    }

    present();

    return 1;
  }
};


int main( int argc, char **argv )
{
  TP app;
  app.run();

  return 0;
}

