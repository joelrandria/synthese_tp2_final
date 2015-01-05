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

#define nb_obj 59

using namespace std;
//! classe utilitaire : permet de construire une chaine de caracteres formatee. cf sprintf.
struct Format
{
    char text[1024];
    
    Format( const char *_format, ... )
    {
        text[0]= 0;     // chaine vide
        // recupere la liste d'arguments supplementaires
        va_list args;
        va_start(args, _format);
        vsnprintf(text, sizeof(text), _format, args);
        va_end(args);
    }
    
    ~Format( ) {}
    
    // conversion implicite de l'objet en chaine de caracteres stantard
    operator const char *( )
    {
        return text;
    }
};


//! squelette d'application gKit.
class TP : public gk::App
{
    nv::SdlContext m_widgets;
    
    gk::GLProgram *m_program;

    int m_indices_size;

    gk::GLBuffer *m_vertex_buffer[nb_obj];
    gk::GLBuffer *m_index_buffer[nb_obj];
    gk::GLVertexArray *m_vao[nb_obj];
    /**/
//    float rotate;
//    float distance;
//    float altitude;
//    float pivote;
    float GD;
    float AA;
    float RY;
    float RX;

    /**/
    gk::GLCounter *m_time;
    
public:
    // creation du contexte openGL et d'une fenetre
    TP( )
        :
        gk::App()
    {
        // specifie le type de contexte openGL a creer :
        gk::AppSettings settings;
        settings.setGLVersion(3,3);     // version 3.3
        settings.setGLCoreProfile();      // core profile
        settings.setGLDebugContext();     // version debug pour obtenir les messages d'erreur en cas de probleme
        
        // cree le contexte et une fenetre
        if(createWindow(512, 512, settings) < 0)
            closeWindow();
        
        m_widgets.init();
        m_widgets.reshape(windowWidth(), windowHeight());
    }
    
    ~TP( ) {}
    
    int init( )
    {
        // compilation simplifiee d'un shader program
        gk::programPath("shaders");
        m_program= gk::createProgram("dFnormal.glsl");//dFnormal.glsl //core.glsl => vertex & fragment shader
        if(m_program == gk::GLProgram::null())
            return -1;


        // charge un mesh//ensemble de triangle

        gk::Mesh *mesh = new gk::Mesh ();
        for(int i = 0 ; i < nb_obj; i++)
        {
                string name;
                ostringstream convert;
                convert << i;
                if(i < 10)
                name = "Bigguy/bigguy_0"+convert.str()+".obj";
                else
                name = "Bigguy/bigguy_"+convert.str()+".obj";

                mesh = gk::MeshIO::readOBJ(name);

            if(mesh == NULL)
               return -1;

            m_vao[i]= gk::createVertexArray();

            // cree le buffer de position
            m_vertex_buffer[i]= gk::createBuffer(GL_ARRAY_BUFFER, mesh->positions);

            glVertexAttribPointer(m_program->attribute("position"), 3, GL_FLOAT, GL_FALSE, 0, 0);

            glEnableVertexAttribArray(m_program->attribute("position"));

            m_index_buffer[i] = gk::createBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indices);
        }

        // conserve le nombre d'indices (necessaire pour utiliser glDrawElements)
        m_indices_size= mesh->indices.size();

        // mesh n'est plus necessaire, les donnees sont transferees dans les buffers sur la carte graphique
        delete mesh;

        // nettoyage de l'etat opengl
        glBindVertexArray(0);   // desactive le vertex array
        glBindBuffer(GL_ARRAY_BUFFER, 0);       // desactive le buffer de positions
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);       // desactive le buffer d'indices
        
        // mesure du temps de dessin
        m_time= gk::createTimer();
        
        /**/
//        distance= -50;
//        rotate= -40;
//        altitude=0;
//        pivote = 0;
        GD = 0;
        AA = -50;
        RY = 0;
        RX = 0;

        /**/
        // ok, tout c'est bien passe
        return 0;
    }
    
    int quit( )
    {
        return 0;
    }

    // a redefinir pour utiliser les widgets.
    void processWindowResize( SDL_WindowEvent& event )
    {
        m_widgets.reshape(event.data1, event.data2);
    }
    
    // a redefinir pour utiliser les widgets.
    void processMouseButtonEvent( SDL_MouseButtonEvent& event )
    {
        m_widgets.processMouseButtonEvent(event);
    }
    
    // a redefinir pour utiliser les widgets.
    void processMouseMotionEvent( SDL_MouseMotionEvent& event )
    {
        m_widgets.processMouseMotionEvent(event);
    }
    
    // a redefinir pour utiliser les widgets.
    void processKeyboardEvent( SDL_KeyboardEvent& event )
    {
        m_widgets.processKeyboardEvent(event);
    }
    
    int draw( )
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
        
        /**/
//        if(key(SDLK_LEFT))
//            rotate+= 0.1f;
//        if(key(SDLK_RIGHT))
//            rotate-= 0.1f;
//        if(key(SDLK_UP))
//            distance+= 0.1f;
//        if(key(SDLK_DOWN))
//            distance-= 0.1f;
//        if(key(SDLK_z))
//            altitude+=0.1f;
//        if(key(SDLK_s))
//            altitude-=0.1f;
//        if(key(SDLK_d))
//            pivote+=0.1f;
//        if(key(SDLK_q))
//            pivote-=0.1f;
        if(key(SDLK_LEFT))
            GD+=0.1f;
        if(key(SDLK_RIGHT))
            GD-=0.1f;
        if(key(SDLK_UP))
            AA+=0.1f;
        if(key(SDLK_DOWN))
            AA-=0.1f;
        if(key(SDLK_d))
            RY-=0.1f;
        if(key(SDLK_q))
            RY+=0.1f;
        if(key(SDLK_z))
            RX+=0.1f;
        if(key(SDLK_s))
            RX-=0.1f;
        /**/

        // redimensionne l'image en fonction de la fenetre de l'application
        glViewport(0, 0, windowWidth(), windowHeight());
        // efface l'image
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // mesurer le temps d'execution
        m_time->start();

        /**/
        // transformations
        gk::Transform model = gk::RotateY(30.f);//distance Zoom
        gk::Transform view =  gk::Translate( gk::Vector(GD, 0.f, AA) ) * gk::RotateX(RX) * gk::RotateY(RY);
        gk::Transform projection= gk::Perspective(50.f, 1.f, 1.f, 1000.f);
        gk::Transform viewport= gk::Viewport(windowWidth(), windowHeight());

        // composition des transformations
        gk::Transform mv= view * model;
        gk::Transform mvp= projection * mv;
        gk::Transform mvpv= viewport * mvp;
        /**/

        // dessiner quelquechose
        glUseProgram(m_program->name);


        // parametrer le shader
        m_program->uniform("mvpMatrix")= gk::Transform().matrix(); // transformation model view projection
   /**/ m_program->uniform("mvpMatrix")= mvp.matrix();
   /**/ m_program->uniform("mvpvInvMatrix")= mvpv.inverseMatrix();
        m_program->uniform("diffuse_color")= gk::VecColor(1, 1, 0);     // couleur des fragments
        
        // selectionner un ensemble de buffers et d'attributs de sommets

       /**********DRAW BOUCLE*******/
        int x = -200;
        int z = 0;
        for(int i = 0; i < nb_obj; i++)
        {
             x += 20;
            if((i % 9) == 0)
            {
                z += 20;
                x = 0;
            }

           glBindVertexArray(m_vao[i]->name);
           glDrawElements(GL_TRIANGLES, m_indices_size, GL_UNSIGNED_INT, 0);
           m_program->uniform("mvpMatrix")= (mvp * gk::Translate(gk::Vector(x, 0.0, z))).matrix();


        }

        // nettoyage

        glUseProgram(0);
        glBindVertexArray(0);
        
        // mesurer le temps d'execution
        m_time->stop();
        
        // afficher le temps d'execution
        {
            m_widgets.begin();
            m_widgets.beginGroup(nv::GroupFlags_GrowDownFromLeft);
            
            m_widgets.doLabel(nv::Rect(), m_time->summary("draw").c_str());
            
            m_widgets.endGroup();
            m_widgets.end();
        }
        
        // afficher le dessin
        present();
        // continuer
        return 1;
    }
};


int main( int argc, char **argv )
{
    TP app;
    app.run();
    
    return 0;
}

