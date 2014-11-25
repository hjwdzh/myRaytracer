#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <OpenGL/gl.h>
#include <OpengL/glu.h>
#include <GLUT/glut.h>

// Load a .BMP file using our custom loader
GLuint loadBMP_custom(const char * imagepath);

//// Since GLFW 3, glfwLoadTexture2D() has been removed. You have to use another texture loading library, 
//// or do it yourself (just like loadBMP_custom and loadDDS)
//// Load a .TGA file using GLFW's own loader
//GLuint loadTGA_glfw(const char * imagepath);

// Load a .DDS file using GLFW's own loader
GLuint loadDDS(const char * imagepath);


GLubyte* grab(const char* img_path, GLubyte* b = 0);

#endif