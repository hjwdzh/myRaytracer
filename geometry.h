#ifndef __GEOMETRY_H_
#define __GEOMETRY_H_

#include <vector>
#include "objloader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
using namespace std;

class Geometry {
public:
	Geometry()
	{
		x_axis = glm::vec4(1,0,0,0);
		y_axis = glm::vec4(0,1,0,0);
		offset = glm::vec3(0,0,0);
		s = glm::vec3(1,1,1);
		rotate = glm::mat4(1.0f);
	}
	Geometry(const char* filename, float kd = 1, float ks = 1, float k3 = 1, bool smooth = false) {
		x_axis = glm::vec4(1,0,0,0);
		y_axis = glm::vec4(0,1,0,0);
		offset = glm::vec3(0,0,0);
		s = glm::vec3(1,1,1);
		rotate = glm::mat4(1.0f);
		if (!loadOBJ(filename, vertex, uv, normal, smooth)) {
			cout << "Load obj fails\n";
		}
		material.x = kd;
		material.y = ks;
		material.z = k3;
	}

	void translate(float x, float y, float z) {
		offset += glm::vec3(x,y,z);
	}

	void translate(const glm::vec3& v) {
		offset += v;
	}

	void rotation(const glm::vec3& axis, float angle) {
		 glm::mat4 mat = glm::rotate(glm::mat4(1.0f), angle, axis);
		 rotate *= mat;
		 x_axis = mat * x_axis;
		 y_axis = mat * y_axis;
	}

	void scale(float sx, float sy, float sz) {
		s *= glm::vec3(sx, sy, sz);
	}

	void scale(const glm::vec3& v) {
		s *= v;
	}

	void applyTransform() {
		glm::mat4 mat = glm::translate(glm::mat4(1.0f), offset);
		glm::mat4 sc = glm::scale(glm::mat4(1.0f), s);
		mat = mat * rotate * sc;
		t_vertex.resize(vertex.size());
		t_normal.resize(normal.size());
		for (int i = 0; i < vertex.size(); ++i) {
			glm::vec4 cur_v = mat * glm::vec4(vertex[i].x, vertex[i].y, vertex[i].z, 1);
			vec3 t_v;
			t_v.x = cur_v.x, t_v.y = cur_v.y, t_v.z = cur_v.z;
			t_vertex[i] = t_v;

			cur_v = rotate * glm::vec4(normal[i].x, normal[i].y, normal[i].z, 0);
			t_v.x = cur_v.x, t_v.y = cur_v.y, t_v.z = cur_v.z;
			t_normal[i] = t_v;
		}
	}

	vector<vec3> vertex, t_vertex;
	vector<vec2> uv;
	vector<vec3> normal, t_normal;
	vec3 material;

	glm::vec3 offset;
	glm::vec4 x_axis;
	glm::vec4 y_axis;
	glm::vec3 s;
	glm::mat4 rotate;
};

#endif