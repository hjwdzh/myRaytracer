#include <iostream>
#include <OpenGL/gl.h>
using namespace std;

extern GLuint programID;

void data_to_uniform(float* t, int d, int size, const char* name) {
	GLuint ID = glGetUniformLocation(programID, name);
	switch (d) {
	case 1:
		glUniform1fv(ID, size, t);
		break;
	case 2:
		glUniform2fv(ID, size, t);
		break;
	case 3:
		glUniform3fv(ID, size, t);
		break;
	}
}

void int_to_uniform(int t, const char* name) {
	GLuint ID = glGetUniformLocation(programID, name);
	glUniform1i(ID, t);
}

GLuint create_texture(float* data, int number) {
	GLuint texture_handle;
    glGenTextures(1, &texture_handle);
    glBindTexture(GL_TEXTURE_1D, texture_handle);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, number, 0, GL_RED, GL_FLOAT, data);
 
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    return texture_handle;
}