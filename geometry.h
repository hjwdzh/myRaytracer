#ifndef __GEOMETRY_H_
#define __GEOMETRY_H_

#include <vector>
#include "objloader.h"
using namespace std;
class Geometry {
public:
	Geometry()
	{
		memset(&x_axis, sizeof(x_axis), 0);
		memset(&y_axis, sizeof(y_axis), 0);
		x_axis.x = 1;
		y_axis.y = 1;
	}

	Geometry(const char* filename, float kd = 1, float ks = 1, float k3 = 1, bool smooth = false) {
		if (!loadOBJ(filename, vertex, uv, normal, smooth)) {
			cout << "Load obj fails\n";
		}
		material.x = kd;
		material.y = ks;
		material.z = k3;
	}

	vector<vec3> vertex;
	vector<vec2> uv;
	vector<vec3> normal;
	vec3 material;

	vec3 offset;
	vec3 x_axis;
	vec3 y_axis;
};

#endif