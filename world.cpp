#include "opengl_util.h"
#include "objloader.h"
#include "sampler.h"
#include "glm/glm.hpp"
#define SAMPLE_SIZE 4

GLuint mesh_texture, normal_texture, material_texture;
int num_triangles, num_object;
float* samples;
extern int g_Width, g_Height;
extern glm::vec3 camera, camera_up, camera_lookat;

void init_scene() {
	static bool initialized = false;
	if (initialized)
		return;
	vector<vec3> vertex, normal;
	vector<vec2> uv;
	if (!loadOBJ("suzanne.obj", vertex, uv, normal)) {
		cout << "Load obj fails\n";
	}
	num_triangles = vertex.size() / 3;
	num_object = 1;
	mesh_texture = create_texture((float*)vertex.data(), num_triangles * 9);
	normal_texture = create_texture((float*)normal.data(), num_triangles * 9);
	float material[] = {1, 1, 1, num_triangles - 0.5};
	material_texture = create_texture(material, 4 * num_object);
	samples = create_sampler(SAMPLE_SIZE);
	initialized = true;
}

//Set the render world
void world() {
	init_scene();
	float viewplane_dis = 1.0;
	float viewplane_scale[] = {g_Width / 480.0, g_Height / 480.0};
	int num_point_light = 2, num_direct_light = 2;
	float point_light[] = {-10, 10, 10, 10, 10, 10};
	float direct_light[] = {0.4 / sqrt(1.32), -1 / sqrt(1.32), -0.4 / sqrt(1.32), -0.4 / sqrt(1.32), -1 / sqrt(1.32), -0.4 / sqrt(1.32)};
	float point_light_color[] = {90, 0, 0, 90, 0, 0};
	float direct_light_color[] = {0.0,0.3, 0, 0, 0.3, 0};
	float ambient[] = {0.0, 0.0, 0.1};

	data_to_uniform(&camera[0], 3, 1, "camera");
	data_to_uniform(&camera_up[0], 3, 1, "camera_up");
	data_to_uniform(&camera_lookat[0], 3, 1, "camera_lookat");
	data_to_uniform(&viewplane_dis, 1, 1, "viewplane_dis");
	data_to_uniform(viewplane_scale, 2, 1, "viewplane_scale");
	int_to_uniform(num_triangles, "num_triangles");
	data_to_uniform(samples, 2, SAMPLE_SIZE * SAMPLE_SIZE, "samples");
	data_to_uniform(point_light, 3, num_point_light, "point_lights");
	data_to_uniform(direct_light, 3, num_direct_light, "direct_lights");
	data_to_uniform(point_light_color, 3, num_point_light, "point_lights_color");
	data_to_uniform(direct_light_color, 3, num_direct_light, "direct_lights_color");
	int_to_uniform(num_point_light, "num_point_light");
	int_to_uniform(num_direct_light, "num_direct_light");
	data_to_uniform(ambient, 3, 1, "ambient");
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, mesh_texture);
	int_to_uniform(0, "meshSampler");
	glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, normal_texture);
	int_to_uniform(1, "normalSampler");
	glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_1D, material_texture);
	int_to_uniform(2, "materialSampler");
	int_to_uniform(num_object, "num_object");
}
