#include "opengl_util.h"

GLuint mesh_texture;
int num_triangles;

void init_scene() {
	static bool initialized = false;
	if (initialized)
		return;
	float triangle[] = {-0.5, -0.5, 4, 0, 0.5, 4, 0.5, -0.5, 4,
						0, -0.5, 5, 0.5, 0.5, 5, 1, -0.5, 5};
	num_triangles = 2;
	mesh_texture = create_texture(triangle, num_triangles * 9);
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, mesh_texture);
	int_to_uniform(0, "meshSampler");
	initialized = true;
}

//Set the render world
void world() {
	init_scene();
	float color[] = {0, 0, -4};
	float viewplane_dis = 1.0;
	float viewplane_scale = 1;
	data_to_uniform(color, 3, 1, "camera");
	data_to_uniform(&viewplane_dis, 1, 1, "viewplane_dis");
	data_to_uniform(&viewplane_scale, 1, 1, "viewplane_scale");
	int_to_uniform(num_triangles, "num_triangles");

}
