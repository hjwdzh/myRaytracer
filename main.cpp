#include <iostream>
#include <GLUT/glut.h>
#include "shader.h"
#include "world.h"
#include "glm/glm.hpp"
#include "opengl_util.h"
#include "texture.h"
#include "sampler.h"
#include <sys/time.h>
using namespace std;

#define SAMPLE_SIZE 8

int g_Width = 600, g_Height = 480, use_bvh = 0, use_path = 0, g_frame = 0;
float g_nearPlane = 0.1, g_farPlane = 5000;
bool rot = false, start_rendering = false;
GLuint renderTexture = -1;
GLubyte* renderData, *data;
char* output_file = 0, *input_file = 0;
extern float* samples;
extern int tex[1];

glm::vec3 camera(0,0,0), camera_up(0,1,0), camera_lookat = glm::normalize(glm::vec3(0,-1,1));

struct timeval last_idle_time, time_now;
enum {
    MENU_LIGHTING = 1,
    MENU_POLYMODE,
    MENU_TEXTURING,
    MENU_EXIT
};

GLuint programID, simpleID;

void DrawTextHHL(const char* text, float x, float y)
{
    const char* c = text;
    
    // 设置字体颜色
    glColor3f(1.0, 1.0, 0.0);
    
    /*
     * 设置正投影
     */
    
    glMatrixMode(GL_PROJECTION);
    
    // 保存当前投影矩阵
    glPushMatrix();
    
    glLoadIdentity();
    gluOrtho2D( 0, g_Width, 0, g_Height );
    
    // 反转Y轴（朝下为正方向）(与窗口坐标一致)
    glScalef(1, -1, 1);
    // 将原点移动到屏幕左上方(与窗口坐标一致)
    glTranslatef(0, -g_Height, 0);
    glMatrixMode(GL_MODELVIEW);
    
    // 保存当前模型视图矩阵
    glPushMatrix();
    glLoadIdentity();
    
    // 设置文字位置
    glRasterPos2f( x, y );
    
    // 依次描绘所有字符(使用显示列表)
    for( ; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    
    // 恢复之前保存的模型视图矩阵
    glPopMatrix();
    
    glMatrixMode(GL_PROJECTION);
    
    // 恢复之前保存的投影矩阵
    glPopMatrix();
    
    // 返回模型视图矩阵状态
    glMatrixMode(GL_MODELVIEW);
}

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
   if (g_frame != (SAMPLE_SIZE*SAMPLE_SIZE))
      RenderObjects();
   // Make sure changes appear onscreen
   glFlush();
   glFinish();
   start_rendering = true;
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
    if (start_rendering) {
      start_rendering = false;
      if (g_frame == 0) {
        renderData = new GLubyte[3 * g_Width * g_Height];
        data = new GLubyte[3 * g_Width * g_Height];
        memset(renderData, 0, sizeof(3 * g_Width * g_Height));
      }
      if (t < 1 / 60.0) {
        sleep(1 / 60.0 - t);
      }
      data = grab(0, data);
      for (int i = 0; i < 3 * g_Width * g_Height; ++i) {
        renderData[i] = (g_frame * (0.0+renderData[i]) + data[i] + 0.0) / (g_frame + 1);
      }
      g_frame++;
      glUseProgram(simpleID);
      // Create one OpenGL texture
      if ((int)renderTexture == -1)
          glGenTextures(1, &renderTexture);
      
      glBindTexture(GL_TEXTURE_2D, renderTexture);

      // Give the image to OpenGL
      glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, g_Width, g_Height, 0, GL_BGR, GL_UNSIGNED_BYTE, renderData);

      // ... nice trilinear filtering.
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
      glGenerateMipmap(GL_TEXTURE_2D);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, renderTexture);
      int_to_uniform(0, "renderSampler", simpleID);
      glClear( GL_COLOR_BUFFER_BIT );
      glBegin(GL_QUADS);
        glVertex3d(-1,-1,0);
        glVertex3d(-1,1,0);
        glVertex3d(1,1,0);
        glVertex3d(1,-1,0);
      glEnd();
      glUseProgram(0);
      char buffer[20];
      sprintf(buffer, "fps %.02f", 1 / t);
      DrawTextHHL(buffer, g_Width * 0.8, 20);
      if (g_frame > SAMPLE_SIZE * SAMPLE_SIZE)
        sprintf(buffer, "Finish!\n");
      else
        sprintf(buffer, "Sampling %d/%d", g_frame + 1, SAMPLE_SIZE * SAMPLE_SIZE);
      DrawTextHHL(buffer, g_Width * 0.75, 40);
      glutSwapBuffers();
    }
    if (g_frame == (SAMPLE_SIZE * SAMPLE_SIZE)) {
        delete[] samples;
        if (output_file) {
          data = grab(output_file);
          delete[] data;
        }
        exit(0);
    }
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
  gettimeofday(&time_now, NULL);
  dt = (float)(time_now.tv_sec  - last_idle_time.tv_sec) +
  1.0e-6*(time_now.tv_usec - last_idle_time.tv_usec);
  last_idle_time = time_now;
#endif
  static int an = 0;
  if (an < 2)
    an++;
  if (an == 2)
    Simulate_To_Frame(dt);
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
    camera = camera + 10.0f * camera_lookat;
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
  case 'r':
    rot = !rot;
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
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-help") == 0) {
      cout << "./render (-path number) (-bvh) -input filename (-output filename)\n";
      exit(0);
    }
    if (strcmp(argv[i], "-bvh") == 0) {
      use_bvh = 1;
    }
    if (strcmp(argv[i], "-path") == 0) {
      use_path = 1;
    }
    if (strcmp(argv[i - 1], "-path") == 0) {
      sscanf(argv[i], "%d", &use_path);      
    }
    if (strcmp(argv[i - 1], "-output") == 0) {
      output_file = new char[100];
      strcpy(output_file, argv[i]);
    }
    if (strcmp(argv[i - 1], "-input") == 0) {
      input_file = new char[100];
      strcpy(input_file, argv[i]);      
    }
  }
  if (input_file == 0) {
    cout << "Please specify input scene file.\n";
    exit(0);
  }
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
  simpleID = LoadShaders("render.vert", "render.frag");
  glutMainLoop ();
  return 0;
}