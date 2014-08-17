#include <iostream>
#include <GLUT/glut.h>
#include "shader.h"
#include "world.h"
#include "glm/glm.hpp"
using namespace std;

int g_Width = 600, g_Height = 480;
float g_nearPlane = 0.1, g_farPlane = 5000;

glm::vec3 camera(0,0,-5), camera_up(0,1,0), camera_lookat(0,0,1);

struct timeval last_idle_time, time_now;
enum {
    MENU_LIGHTING = 1,
    MENU_POLYMODE,
    MENU_TEXTURING,
    MENU_EXIT
};

GLuint programID;

void RenderObjects(void)
{
    world();
    glClear( GL_COLOR_BUFFER_BIT );
    glUseProgram(programID);
    glBegin(GL_QUADS);
      glVertex3d(-1,-1,0);
      glVertex3d(-1,1,0);
      glVertex3d(1,1,0);
      glVertex3d(1,-1,0);
    glEnd();
}

void display(void)
{
   // Clear frame buffer and depth buffer
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   // Set up viewing transformation, looking down -Z axis
   glLoadIdentity();
   // Render the scene
   RenderObjects();
   // Make sure changes appear onscreen
   glutSwapBuffers();
}

void reshape(GLint width, GLint height)
{
   g_Width = width;
   g_Height = height;
   glViewport(0, 0, g_Width, g_Height);
}

void MouseButton(int button, int state, int x, int y)
{
  // Respond to mouse button presses.
  // If button1 pressed, mark this state so we know in motion function.
}

void MouseMotion(int x, int y)
{
  // If button1 pressed, zoom in/out if mouse is moved up/down.
}

void Simulate_To_Frame(float t)
{

}

void AnimateScene(void)
{
  float dt;
#ifdef _WIN32
  DWORD time_now;
  time_now = GetTickCount();
  dt = (float) (time_now - last_idle_time) / 1000.0;
#else
  // Figure out time elapsed since last call to idle function
  struct timeval time_now;
//  gettimeofday(&time_now, NULL);
  dt = (float)(time_now.tv_sec  - last_idle_time.tv_sec) +
  1.0e-6*(time_now.tv_usec - last_idle_time.tv_usec);
#endif
  Simulate_To_Frame(dt);
  // Save time_now for next time
  last_idle_time = time_now;
  // Force redraw
  glutPostRedisplay();
}

void SelectFromMenu(int idCommand)
{
  switch (idCommand)
    {
    case MENU_LIGHTING:
      break;
    case MENU_POLYMODE:
      break;      
    case MENU_TEXTURING:
      break;    
    case MENU_EXIT:
      exit (0);
      break;
    }
  // Almost any menu selection requires a redraw
  glutPostRedisplay();
}

void Keyboard(unsigned char key, int x, int y)
{
  glm::vec3 camera_x = glm::cross(camera_lookat, camera_up);
  switch (key)
  {
  case 27:             // ESCAPE key
	  exit (0);
	  break;
  case 'w':
    camera_up = glm::normalize(camera_up - (float)(tan(5 / 180.0 * 3.1415926)) * camera_lookat);
    camera_lookat = glm::cross(camera_up, camera_x);
    break;
  case 's':
    camera_up = glm::normalize(camera_up + (float)(tan(5 / 180.0 * 3.1415926)) * camera_lookat);
    camera_lookat = glm::cross(camera_up, camera_x);
    break;
  case 'd':
    camera_lookat = glm::normalize(camera_lookat + (float)(tan(5 / 180.0 * 3.1415926)) * camera_x);
    break;
  case 'a':
    camera_lookat = glm::normalize(camera_lookat - (float)(tan(5 / 180.0 * 3.1415926)) * camera_x);
    break;
  case 'f':
    camera = camera + 0.1f * camera_lookat;
    break;
  case 'b':
    camera = camera - 0.1f * camera_lookat;
    break;
  case 'u':
    camera = camera + 0.1f * camera_up;
    break;
  case 'n':
    camera = camera - 0.1f * camera_up;
    break;
  case 'i':
    camera = camera + 0.1f * camera_x;
    break;
  case 'k':
    camera = camera - 0.1f * camera_x;
    break;
  case 'l':
	  SelectFromMenu(MENU_LIGHTING);
	  break;
  case 'p':
	  SelectFromMenu(MENU_POLYMODE);
	  break;
  case 't':
	  SelectFromMenu(MENU_TEXTURING);
	  break;
  }
}

int BuildPopupMenu (void)
{
  int menu;
  menu = glutCreateMenu (SelectFromMenu);
  glutAddMenuEntry ("Toggle lighting\tl", MENU_LIGHTING);
  glutAddMenuEntry ("Toggle polygon fill\tp", MENU_POLYMODE);
  glutAddMenuEntry ("Toggle texturing\tt", MENU_TEXTURING);
  glutAddMenuEntry ("Exit demo\tEsc", MENU_EXIT);
  return menu;
}

int main(int argc, char** argv)
{
  // GLUT Window Initialization:
  glutInit (&argc, argv);
  glutInitWindowSize (g_Width, g_Height);
  glutInitDisplayMode ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow ("Raytracer");
  // Initialize OpenGL graphics state
  // Register callbacks:
  glutDisplayFunc (display);
  glutReshapeFunc (reshape);
  glutKeyboardFunc (Keyboard);
  glutMouseFunc (MouseButton);
  glutMotionFunc (MouseMotion);
  glutIdleFunc (AnimateScene);
  // Create our popup menu
  BuildPopupMenu ();
  glutAttachMenu (GLUT_RIGHT_BUTTON);
  // Get the initial time, for use by animation
  // Turn the flow of control over to GLUT
  programID = LoadShaders( "raytrace.vert", "raytrace.frag" );
  glutMainLoop ();
  return 0;
}