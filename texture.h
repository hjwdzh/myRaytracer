#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <OpenGL/gl.h>
#include <OpengL/glu.h>
#include <GLUT/glut.h>
#include <map>
#include <vector>
#include <string>
using namespace std;
// Load a .BMP file using our custom loader
GLuint loadBMP_custom(const char * imagepath);

//// Since GLFW 3, glfwLoadTexture2D() has been removed. You have to use another texture loading library, 
//// or do it yourself (just like loadBMP_custom and loadDDS)
//// Load a .TGA file using GLFW's own loader
//GLuint loadTGA_glfw(const char * imagepath);

// Load a .DDS file using GLFW's own loader
GLuint loadDDS(const char * imagepath);


GLubyte* grab(const char* img_path, GLubyte* b = 0);

class TexManager {
public:
	static map<string,GLuint> nameManager;
	static vector<GLuint> texResource;
	static GLuint createRenderTexture(const char* name) {
		string n = string(name);
		map<string,GLuint>::iterator it = nameManager.find(n);
		GLuint id;
		if (it == nameManager.end()) {
			id = loadBMP_custom((string("tex/")+name).c_str());
			texResource.push_back(id);
			id = texResource.size() - 1;
			nameManager.insert(pair<string,GLuint>(n, id));
		} else {
			id = it->second;
		}
		return id;
	}
};

#endif