#ifndef __GEOMETRY_H_
#define __GEOMETRY_H_

#include <vector>
#include "objloader.h"

using namespace std;
class Geometry {
public:
	Geometry()
	{
		memset(&x_axis, 0, sizeof(x_axis));
		memset(&y_axis, 0, sizeof(y_axis));
		memset(&offset, 0, sizeof(offset));
		x_axis.x = 1;
		y_axis.y = 1;
	}

	Geometry(const char* filename, float kd = 1, float ks = 1, float k3 = 1, bool smooth = false) {
		if (!loadOBJ(filename, vertex, uv, normal, smooth)) {
			cout << "Load obj fails\n";
		}
		memset(&x_axis, 0, sizeof(x_axis));
		memset(&y_axis, 0, sizeof(y_axis));
		memset(&offset, 0, sizeof(x_axis));
		x_axis.x = 1;
		y_axis.y = 1;
		material.x = kd;
		material.y = ks;
		material.z = k3;
	}

	void Translate(const glm::vec3& v) {
		offset.x += v.x;
		offset.y += v.y;
		offset.z += v.z;
	}

	void Scale(float s) {
		s = 1 / s;
		x_axis.x *= s;
		x_axis.y *= s;
		x_axis.z *= s;
		y_axis.x *= s;
		y_axis.y *= s;
		y_axis.z *= s;
	}

	void Rotate(float angle, const glm::vec3& axis, vec3& target) {
		angle = angle / 180.0 * 3.141592654;
		glm::vec3 orig(target.x, target.y, target.z);
		glm::vec3 y_axis = glm::cross(axis, orig);
		glm::vec3 x_axis = glm::cross(y_axis, axis);
		glm::vec3 orig_z = glm::dot(orig, axis) * axis;
		orig_z += glm::length(orig - orig_z) * (cos(angle) * x_axis + sin(angle) * y_axis);
		target.x = orig_z[0];
		target.y = orig_z[1];
		target.z = orig_z[2];
	}

	void Rotate(float angle, const glm::vec3& axis) {
		Rotate(angle, axis, x_axis);
		Rotate(angle, axis, y_axis);
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