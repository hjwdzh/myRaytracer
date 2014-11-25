#include "opengl_util.h"

void data_to_uniform(float* t, int d, int size, const char* name, GLuint id) {
	GLuint ID = glGetUniformLocation(id, name);
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

void data_to_uniform(int* t, int d, int size, const char* name, GLuint id) {
	GLuint ID = glGetUniformLocation(id, name);
	switch (d) {
	case 1:
		glUniform1iv(ID, size, t);
		break;
	case 2:
		glUniform2iv(ID, size, t);
		break;
	case 3:
		glUniform3iv(ID, size, t);
		break;
	}
}

void int_to_uniform(int t, const char* name, GLuint id) {
	GLuint ID = glGetUniformLocation(id, name);
	glUniform1i(ID, t);
}

GLuint create_texture(float* data, int number, GLuint texture_handle) {
	if ((int)texture_handle == -1)
	    glGenTextures(1, &texture_handle);
    glBindTexture(GL_TEXTURE_1D, texture_handle);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, number, 0, GL_RED, GL_FLOAT, data);
 
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    return texture_handle;
}

GLuint create_texture_2D(float* data, int dim, int number, GLuint texture_handle) {
	if ((int)texture_handle == -1) {
	    glGenTextures(1, &texture_handle);
	}
    glBindTexture(GL_TEXTURE_2D, texture_handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, dim, number, 0, GL_RED, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    return texture_handle;
}