#ifndef OBJLOADER_H
#define OBJLOADER_H
#include <vector>
class Geometry;
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
	std::vector<vec3> & out_normals,
	bool smooth
);

bool loadScene(
	const char * path, std::vector<Geometry*>& geometries
);

#endif