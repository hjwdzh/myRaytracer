#include "opengl_util.h"
#include "objloader.h"
#include "sampler.h"

#define SAMPLE_SIZE 4

GLuint mesh_texture;
int num_triangles;
float* samples;
extern int g_Width, g_Height;

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
	mesh_texture = create_texture((float*)vertex.data(), num_triangles * 9);
	samples = create_sampler(SAMPLE_SIZE);
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, mesh_texture);
	int_to_uniform(0, "meshSampler");
	initialized = true;
}

//Set the render world
void world() {
	init_scene();
	float camera[] = {0, 0, -2};
	float viewplane_dis = 1.0;
	float viewplane_scale[] = {g_Width / 480.0, g_Height / 480.0};
	data_to_uniform(camera, 3, 1, "camera");
	data_to_uniform(&viewplane_dis, 1, 1, "viewplane_dis");
	data_to_uniform(viewplane_scale, 2, 1, "viewplane_scale");
	int_to_uniform(num_triangles, "num_triangles");
	data_to_uniform(samples, 2, SAMPLE_SIZE * SAMPLE_SIZE, "samples");
}
