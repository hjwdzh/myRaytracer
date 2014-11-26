#ifndef LIGHT_H_
#define LIGHT_H_

#include <vector>
#include "objloader.h"

class Light {
public:
	std::vector<vec3> point_light_pos;
	std::vector<vec3> point_light_color;
	std::vector<vec3> direct_light_dir;
	std::vector<vec3> direct_light_color;
	vec3 ambient;
};

#endif