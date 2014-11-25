#ifndef BVH_H_
#define BVH_H_

#include "objloader.h"
#include <vector>
#include "glm/glm.hpp"
#include "math.h"
using namespace std;

class BBox {
public:
	BBox() {
		memset(&min_corner, 0, sizeof(min_corner));
		memset(&max_corner, 0, sizeof(max_corner));
	}
	BBox(float* vertices) {
		min_corner.x = min(min(vertices[0], vertices[3]), vertices[6]);
		min_corner.y = min(min(vertices[1], vertices[4]), vertices[7]);
		min_corner.z = min(min(vertices[2], vertices[5]), vertices[8]);
		max_corner.x = max(max(vertices[0], vertices[3]), vertices[6]);
		max_corner.y = max(max(vertices[1], vertices[4]), vertices[7]);
		max_corner.z = max(max(vertices[2], vertices[5]), vertices[8]);
	}
	void Append(const BBox& b) {
		min_corner.x = min(min_corner.x, b.min_corner.x);
		min_corner.y = min(min_corner.y, b.min_corner.y);
		min_corner.z = min(min_corner.z, b.min_corner.z);
		max_corner.x = max(max_corner.x, b.max_corner.x);
		max_corner.y = max(max_corner.y, b.max_corner.y);
		max_corner.z = max(max_corner.z, b.max_corner.z);
	}
	glm::vec3 mins() {
		return glm::vec3(min_corner.x, min_corner.y, min_corner.z);
	}
	glm::vec3 maxs() {
		return glm::vec3(max_corner.x, max_corner.y, max_corner.z);
	}
	vec3 min_corner, max_corner;
};

class BVH {
public:
	BVH(){}
	BVH(float* vertices) {
		bbox = BBox(vertices);
		left = 0; right = 0;
	}
	BVH(float* vertices, float* normals, float* uv, float* indices, int num, int dim = 0, int level = 0) {
		this->axis = dim;
		if (num == 1) {
			bbox = BBox(vertices);
			left = 0; right = 0;
			return;
		}
		bbox = BBox(vertices);
		for (int i = 1; i < num; i++) {
			bbox.Append(BBox(vertices + 9 * i));
		}
		glm::vec3 pivot = 0.5f * (bbox.maxs() + bbox.mins());
		mid_point = qsplit(vertices, normals, uv, indices, num, pivot[dim], dim);
		left = right = 0;
		if (mid_point > 0)
			left = new BVH(vertices, normals, uv, indices, mid_point, (dim + 1) % 3, level + 1);
		if (num - mid_point > 0)
			right = new BVH(vertices + 9 * mid_point, normals + 9 * mid_point, uv + 6 * mid_point, indices + mid_point, num - mid_point, (dim + 1) % 3, level + 1);
	}
	void genBuffer(vector<float>& buffer, int start = 0, int start_ind = 0, int parent_id = -1) {
		buffer.resize(11);
		buffer[0] = bbox.min_corner.x;
		buffer[1] = bbox.min_corner.y;
		buffer[2] = bbox.min_corner.z;
		buffer[3] = bbox.max_corner.x;
		buffer[4] = bbox.max_corner.y;
		buffer[5] = bbox.max_corner.z;
		buffer[6] = this->axis;
		buffer[7] = start_ind;
		buffer[8] = buffer[9] = -1;
		vector<float> left_buffer, right_buffer;

		if (left) {
			buffer[8] = start + 1;
			left->genBuffer(left_buffer, buffer[8], start_ind, start);
		}
		if (right) {
			buffer[9] = start + 1 + left_buffer.size() / 11;
			right->genBuffer(right_buffer, buffer[9], start_ind + mid_point, start);
		}
		buffer[10] = parent_id;
		buffer.insert(buffer.end(), left_buffer.begin(), left_buffer.end());
		buffer.insert(buffer.end(), right_buffer.begin(), right_buffer.end());
	}
	int qsplit(float* vertices, float* normals, float* uv, float* indices, int size, float pivot_val, int axis) {
		int i = 0, j = size - 1;
		float centroid;
		float temp[9];
		BBox bbox;
		while (i < j) {
			bbox = BBox(vertices + i * 9);
			centroid = (bbox.mins()[axis] + bbox.maxs()[axis]) * 0.5;
			while (i <= j && centroid <= pivot_val) {
				++i;
				if (i <= j) {
					bbox = BBox(vertices + i * 9);
					centroid = (bbox.mins()[axis] + bbox.maxs()[axis]) * 0.5;
				}
			}
			break;
			bbox = BBox(vertices + j * 9);
			centroid = (bbox.mins()[axis] + bbox.maxs()[axis]) * 0.5;			
			while (j >= i && centroid >= pivot_val) {
				--j;
				if (j >= i) {
					bbox = BBox(vertices + j * 9);
					centroid = (bbox.mins()[axis] + bbox.maxs()[axis]) * 0.5;			
				}
			}
			if (i <= j) {
				memcpy(temp, vertices + i * 9, 9 * sizeof(float));
				memcpy(vertices + i * 9, vertices + j * 9, 9 * sizeof(float));
				memcpy(vertices + j * 9, temp, 9 * sizeof(float));
				memcpy(temp, normals + i * 9, 9 * sizeof(float));
				memcpy(normals + i * 9, normals + j * 9, 9 * sizeof(float));
				memcpy(normals + j * 9, temp, 9 * sizeof(float));
				memcpy(temp, uv + i * 6, 6 * sizeof(float));
				memcpy(uv + i * 6, uv + j * 6, 6 * sizeof(float));
				memcpy(uv + j * 6, temp, 6 * sizeof(float));
				int tmp = indices[i];
				indices[i] = indices[j];
				indices[j] = tmp;
				++i, --j;
			}
		}
		if (i >= size || i == 0) {
			i = size / 2;
		}
		return i;
	}
	BBox bbox;
	BVH *left, *right;
	int axis;
	int mid_point;
};

#endif