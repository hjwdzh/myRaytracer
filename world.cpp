#include "opengl_util.h"
#include "objloader.h"
#include "sampler.h"
#include "glm/glm.hpp"
#include "Geometry.h"
#include "BVH.h"
#include <fstream>
#define SAMPLE_SIZE 4

GLuint mesh_texture, normal_texture, material_texture, mat_index_texture, 
	min_coord_texture, max_coord_texture, bvh_index_texture;
int num_triangles, num_object, num_bvh;
float* samples;
extern int g_Width, g_Height;
extern glm::vec3 camera, camera_up, camera_lookat;

void generate_geometries(vector<Geometry*>& geometries) {
	num_triangles = 0;
	num_object = geometries.size();
	for (int i = 0; i < num_object; ++i)
		num_triangles += geometries[i]->vertex.size() / 3;
	float* vertex_buffer = new float[num_triangles * 9];
	float* normal_buffer = new float[num_triangles * 9];
	float* material = new float[13 * num_object];
	float* mat_index = new float[num_triangles];
	vector<float> min_coord, max_coord, bvh_index;
	float *v_ptr = vertex_buffer, *n_ptr = normal_buffer, 
		  *m_ptr = material;
	int s = 0;
	for (int i = 0; i < num_object; ++i) {
		int num_tri = geometries[i]->vertex.size() / 3;
		for (int j = 0; j < num_tri; ++j)
			mat_index[s + j] = i;
		memcpy(v_ptr, geometries[i]->vertex.data(), geometries[i]->vertex.size() * 3 * sizeof(float));
		v_ptr += geometries[i]->vertex.size() * 3;
		memcpy(n_ptr, geometries[i]->normal.data(), geometries[i]->normal.size() * 3 * sizeof(float));
		n_ptr += geometries[i]->normal.size() * 3;
		memcpy(m_ptr, &(geometries[i]->material), sizeof(float) * 3);
		m_ptr += 3;
		memcpy(m_ptr, &(geometries[i]->offset), sizeof(float) * 3);
		m_ptr += 3;
		memcpy(m_ptr, &(geometries[i]->x_axis), sizeof(float) * 3);
		m_ptr += 3;
		memcpy(m_ptr, &(geometries[i]->y_axis), sizeof(float) * 3);
		m_ptr += 3;
		s += geometries[i]->vertex.size() / 3;
		*m_ptr++ = s;
	}
	BVH bvh(vertex_buffer, normal_buffer, mat_index, num_triangles);
	num_bvh = bvh.generate_data(min_coord, max_coord, bvh_index);
	mesh_texture = create_texture(vertex_buffer, num_triangles * 9);
	normal_texture = create_texture(normal_buffer, num_triangles * 9);
	material_texture = create_texture(material, 13 * num_object);
	mat_index_texture = create_texture(mat_index, num_triangles);
	min_coord_texture = create_texture(min_coord.data(), num_bvh * 3);
	max_coord_texture = create_texture(max_coord.data(), num_bvh * 3);
	bvh_index_texture = create_texture(bvh_index.data(), num_bvh * 2);
	delete[] vertex_buffer;
	delete[] normal_buffer;
	delete[] material;
	delete[] mat_index;
}

void init_scene() {
	static bool initialized = false;
	if (initialized)
		return;
	vector<Geometry*> geometries;
	Geometry* g = new Geometry("cube.obj",0,0,0);
	Geometry* g1 = new Geometry("plane.obj");
//	g->Translate(glm::vec3(1,1,1));
//	g->Rotate(90, glm::vec3(1/sqrt(3),1/sqrt(3),1/sqrt(3)));
	geometries.push_back(g);
	geometries.push_back(g1);
	generate_geometries(geometries);
	samples = create_sampler(SAMPLE_SIZE);
	initialized = true;
}

//Set the render world
void world() {
	init_scene();
	float viewplane_dis = 1.0;
	float viewplane_scale[] = {g_Width / 480.0, g_Height / 480.0};
	int num_point_light = 2, num_direct_light = 2;
	float point_light[] = {20, 20, -20, -20, 20, -20};
	float direct_light[] = {0.4 / sqrt(1.32), -1 / sqrt(1.32), -0.4 / sqrt(1.32), -0.4 / sqrt(1.32), -1 / sqrt(1.32), -0.4 / sqrt(1.32)};
	float point_light_color[] = {300, 0, 0, 300, 0, 0};
	float direct_light_color[] = {0.0,0.3, 0, 0, 0.3, 0};
	float ambient[] = {0.0, 0.0, 0.1};

	data_to_uniform(&camera[0], 3, 1, "camera");
	data_to_uniform(&camera_up[0], 3, 1, "camera_up");
	data_to_uniform(&camera_lookat[0], 3, 1, "camera_lookat");
	data_to_uniform(&viewplane_dis, 1, 1, "viewplane_dis");
	data_to_uniform(viewplane_scale, 2, 1, "viewplane_scale");
	data_to_uniform(samples, 2, SAMPLE_SIZE * SAMPLE_SIZE, "samples");
	data_to_uniform(point_light, 3, num_point_light, "point_lights");
	data_to_uniform(direct_light, 3, num_direct_light, "direct_lights");
	data_to_uniform(point_light_color, 3, num_point_light, "point_lights_color");
	data_to_uniform(direct_light_color, 3, num_direct_light, "direct_lights_color");
	data_to_uniform(ambient, 3, 1, "ambient");

	int_to_uniform(num_triangles, "num_triangles");
	int_to_uniform(num_point_light, "num_point_light");
	int_to_uniform(num_direct_light, "num_direct_light");
	int_to_uniform(num_object, "num_object");
	int_to_uniform(num_bvh, "num_bvh");

	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, mesh_texture);
	int_to_uniform(0, "meshSampler");
	glActiveTexture(GL_TEXTURE0+1);
    glBindTexture(GL_TEXTURE_1D, normal_texture);
	int_to_uniform(1, "normalSampler");
	glActiveTexture(GL_TEXTURE0+2);
    glBindTexture(GL_TEXTURE_1D, material_texture);
	int_to_uniform(2, "materialSampler");
	glActiveTexture(GL_TEXTURE0+3);
    glBindTexture(GL_TEXTURE_1D, mat_index_texture);
	int_to_uniform(3, "matIndexSampler");
	glActiveTexture(GL_TEXTURE0+4);
    glBindTexture(GL_TEXTURE_1D, min_coord_texture);
	int_to_uniform(4, "minCoordSampler");
	glActiveTexture(GL_TEXTURE0+5);
    glBindTexture(GL_TEXTURE_1D, max_coord_texture);
	int_to_uniform(5, "maxCoordSampler");
	glActiveTexture(GL_TEXTURE0+6);
    glBindTexture(GL_TEXTURE_1D, bvh_index_texture);
	int_to_uniform(6, "bvhIndexSampler");
}
