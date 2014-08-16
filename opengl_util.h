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