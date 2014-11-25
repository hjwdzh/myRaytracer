#ifndef OPENGL_UTIL_H_
#define OPENGL_UTIL_H_

#include <iostream>
#include <OpenGL/gl.h>
using namespace std;

extern GLuint programID;

void data_to_uniform(float* t, int d, int size, const char* name, GLuint id = programID);

void data_to_uniform(int* t, int d, int size, const char* name, GLuint id = programID);

void int_to_uniform(int t, const char* name, GLuint id = programID);

GLuint create_texture(float* data, int number, GLuint texture_handle);

GLuint create_texture_2D(float* data, int dim, int number, GLuint texture_handle);

#endif