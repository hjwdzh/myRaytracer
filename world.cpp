#include "opengl_util.h"
#include "objloader.h"
#include "sampler.h"
#include "glm/glm.hpp"
#include "geometry.h"
#include "texture.h"
#include "BVH.h"
#include "light.h"
#include <fstream>
#define SAMPLE_SIZE 8

GLuint mesh_texture = -1, normal_texture = -1, material_texture = -1, tex_texture = -1, index_texture = -1, bvh_texture = -1, sampler_texture = -1;
int num_triangles, num_object, num_bvh;
Light lights;
float* samples;
extern int g_Width, g_Height, use_bvh, g_frame, use_path;
extern bool rot;
extern glm::vec3 camera, camera_up, camera_lookat;
extern char* input_file;
vector<Geometry*> geometries;

void generate_geometries(vector<Geometry*>& geometries) {
	num_triangles = 0;
	num_object = geometries.size();
	for (int i = 0; i < num_object; ++i)
		num_triangles += geometries[i]->vertex.size() / 3;
	float* vertex_buffer = new float[num_triangles * 9];
	float* normal_buffer = new float[num_triangles * 9];
	float* tex_buffer = new float[num_triangles * 6];
	float* index_buffer = new float[num_triangles];
	float* material = new float[21 * num_object];
	float* v_ptr = vertex_buffer, *n_ptr = normal_buffer, *m_ptr = material, *t_ptr = tex_buffer;
	int s = 0;
	for (int i = 0; i < num_object; ++i) {
		if (use_bvh) {
			geometries[i]->applyTransform();
			memcpy(v_ptr, geometries[i]->t_vertex.data(), geometries[i]->t_vertex.size() * 3 * sizeof(float));
			v_ptr += geometries[i]->vertex.size() * 3;
			memcpy(n_ptr, geometries[i]->t_normal.data(), geometries[i]->t_normal.size() * 3 * sizeof(float));
			n_ptr += geometries[i]->t_normal.size() * 3;
		} else {
			memcpy(v_ptr, geometries[i]->vertex.data(), geometries[i]->vertex.size() * 3 * sizeof(float));
			v_ptr += geometries[i]->vertex.size() * 3;
			memcpy(n_ptr, geometries[i]->normal.data(), geometries[i]->normal.size() * 3 * sizeof(float));
			n_ptr += geometries[i]->normal.size() * 3;			
		}
		memcpy(t_ptr, geometries[i]->uv.data(), geometries[i]->uv.size() * 2 * sizeof(float));
		t_ptr += geometries[i]->uv.size() * 2;
		*m_ptr++ = geometries[i]->kd;
		*m_ptr++ = geometries[i]->ks;
		*m_ptr++ = geometries[i]->tex;
		*m_ptr++ = geometries[i]->offset.x;
		*m_ptr++ = geometries[i]->offset.y;
		*m_ptr++ = geometries[i]->offset.z;
		*m_ptr++ = geometries[i]->x_axis.x;
		*m_ptr++ = geometries[i]->x_axis.y;
		*m_ptr++ = geometries[i]->x_axis.z;
		*m_ptr++ = geometries[i]->y_axis.x;
		*m_ptr++ = geometries[i]->y_axis.y;
		*m_ptr++ = geometries[i]->y_axis.z;
		*m_ptr++ = geometries[i]->s.x;
		*m_ptr++ = geometries[i]->s.y;
		*m_ptr++ = geometries[i]->s.z;
		for (int j = s / 3; j < s / 3 + geometries[i]->vertex.size() / 3; ++j)
			index_buffer[j] = i;
		s += geometries[i]->vertex.size();
		*m_ptr++ = s;
		*m_ptr++ = geometries[i]->ka;
		*m_ptr++ = geometries[i]->kr;
		*m_ptr++ = geometries[i]->kf;
		*m_ptr++ = geometries[i]->nr;
		*m_ptr++ = geometries[i]->alpha;
	}
	BVH* bvh = new BVH(vertex_buffer, normal_buffer, tex_buffer, index_buffer, num_triangles);
	vector<float> bvh_buffer;
	bvh->genBuffer(bvh_buffer);
	mesh_texture = create_texture_2D(vertex_buffer, 9, num_triangles, mesh_texture);
	normal_texture = create_texture_2D(normal_buffer, 9, num_triangles, normal_texture);
	material_texture = create_texture_2D(material, 21, num_object, material_texture);
	tex_texture = create_texture_2D(tex_buffer, 6, num_triangles, tex_texture);
	index_texture = create_texture(index_buffer, num_triangles, index_texture);
	bvh_texture = create_texture_2D(bvh_buffer.data(), 11, bvh_buffer.size() / 11, bvh_texture);
	sampler_texture = create_texture(samples, 2 * SAMPLE_SIZE * SAMPLE_SIZE + 2, sampler_texture);
	num_bvh = bvh_buffer.size() / 11;
	delete[] vertex_buffer;
	delete[] normal_buffer;
	delete[] tex_buffer;
	delete[] material;
}

void init_scene() {
	static bool initialized = false;
	if (initialized)
		return;
	if (!loadScene(input_file, geometries)) {
		exit(0);
	}
	samples = create_sampler(SAMPLE_SIZE);
	initialized = true;
}

//Set the render world
void world() {
	init_scene();
	samples[SAMPLE_SIZE*SAMPLE_SIZE*2] = g_frame % (SAMPLE_SIZE * SAMPLE_SIZE);
	generate_geometries(geometries);
	float viewplane_dis = 1.0;
	float viewplane_scale[] = {g_Width / 480.0, g_Height / 480.0};
	int num_point_light = lights.point_light_pos.size(), num_direct_light = lights.direct_light_dir.size();
	data_to_uniform(&camera[0], 3, 1, "camera");
	data_to_uniform(&camera_up[0], 3, 1, "camera_up");
	data_to_uniform(&camera_lookat[0], 3, 1, "camera_lookat");
	data_to_uniform(&viewplane_dis, 1, 1, "viewplane_dis");
	data_to_uniform(viewplane_scale, 2, 1, "viewplane_scale");
	int_to_uniform(num_triangles, "num_triangles");
	data_to_uniform((float*)lights.point_light_pos.data(), 3, num_point_light, "point_lights");
	data_to_uniform((float*)lights.direct_light_dir.data(), 3, num_direct_light, "direct_lights");
	data_to_uniform((float*)lights.point_light_color.data(), 3, num_point_light, "point_lights_color");
	data_to_uniform((float*)lights.direct_light_color.data(), 3, num_direct_light, "direct_lights_color");
	int_to_uniform(num_point_light, "num_point_light");
	int_to_uniform(num_direct_light, "num_direct_light");
	data_to_uniform((float*)&(lights.ambient), 3, 1, "ambient");
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mesh_texture);
	int_to_uniform(0, "meshSampler");
	glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normal_texture);
	int_to_uniform(1, "normalSampler");
	glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, material_texture);
	int_to_uniform(2, "materialSampler");
	glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, tex_texture);
	int_to_uniform(3, "texSampler");
	glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_1D, index_texture);
	int_to_uniform(4, "indexSampler");
	glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, bvh_texture);
	int_to_uniform(5, "bvhSampler");	
	glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_1D, sampler_texture);
	int_to_uniform(6, "samples");
	int* tex_id = new int[TexManager::texResource.size()];
	for (int i = 0; i < TexManager::texResource.size(); ++i) {
		tex_id[i] = 7 + i;
		glActiveTexture(GL_TEXTURE0 + 7 + i);
		glBindTexture(GL_TEXTURE_2D, TexManager::texResource[i]);
	}
	data_to_uniform(tex_id, 1, TexManager::texResource.size(), "renderSampler");
//	int_to_uniform(7, "renderSampler");
	int_to_uniform(num_object, "num_object");
	int_to_uniform(num_bvh, "num_bvh");
	int_to_uniform(use_bvh, "use_bvh");
	int_to_uniform(use_path, "use_path");
	delete[] tex_id;
//	data_to_uniform(tex, 1, 1, "renderSampler");
}
