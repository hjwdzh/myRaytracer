#ifndef _BVH_H_
#define _BVH_H_

#include "objloader.h"
#include <vector>
#include "glm/glm.hpp"
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
      return glm::vec3(min_corner.x,min_corner.y,min_corner.z);
   }
   glm::vec3 maxs() {
      return glm::vec3(max_corner.x,max_corner.y,max_corner.z);
   }
   vec3 min_corner, max_corner;
};

class BVH
{
public:
   BVH()
   {}
   BVH(float* vertices, int* indices) {
      bbox = BBox(vertices);
      is_leaf = 1;
      index = vertices;
      material = indices[0];
   }
   BVH(float* vertices, float* normals, int* indices, int num, int dim = 0)
   {
      if (num == 1) {
            *this = BVH(vertices, indices);
            return;
      }

      is_leaf = 0;

      bbox = BBox(vertices);
      for (int i = 1; i < num; i++)
         bbox.Append(vertices + 9 * i);

      glm::vec3 pivot = (bbox.maxs() + bbox.mins()) / 2.0f;
      
      int mid_point = qsplit(vertices, normals, indices, num, pivot[dim], dim);
      // create a new boundingVolume
      left  = new BVH(vertices, normals, indices, mid_point, (dim + 1) % 3);
      right = new BVH(vertices + 9 * mid_point, normals + 9 * mid_point, indices + mid_point, num - mid_point, (dim + 1) % 3);
   }
   int qsplit(float* vertices, float* normals, int* indices, int size, float pivot_val, int axis)
   {
      BBox bbox;
      float centroid;
      float temp[9];
      int ret_val = 0;

      for (int i = 0; i < size; i++)
      {
         bbox = BBox(vertices + i * 9);
         centroid = ((bbox.mins())[axis] + (bbox.maxs())[axis]) / 2.0f;
         if (centroid < pivot_val)
         {
            memcpy(temp, vertices + i * 9, 9 * sizeof(float));
            memcpy(vertices + i * 9, vertices + ret_val * 9, 9 * sizeof(float));
            memcpy(vertices + ret_val * 9, temp, 9 * sizeof(float));
            memcpy(temp, normals + i * 9, 9 * sizeof(float));
            memcpy(normals + i * 9, normals + ret_val * 9, 9 * sizeof(float));
            memcpy(normals + ret_val * 9, temp, 9 * sizeof(float));
            int tmp = indices[i];
            indices[i] = indices[ret_val];
            indices[ret_val] = tmp;
            ret_val++;
         }
      }
      if (ret_val == 0 || ret_val == size) ret_val = size/2;

      return ret_val;
   }
   BBox bbox;
   float *index;
   int material;
   BVH *left, *right;
   int is_leaf;
};

#endif // _BVH_H_
