#ifndef OBJLOADER_H
#define OBJLOADER_H
#include <vector>

struct vec3 {
	float x, y, z;
};

struct vec2 {
	float x, y;
};

bool loadOBJ(
	const char * path, 
	std::vector<vec3> & out_vertices, 
	std::vector<vec2> & out_uvs, 
	std::vector<vec3> & out_normals
);

#endif