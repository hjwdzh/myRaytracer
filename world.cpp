#include "opengl_util.h"

//Set the render world
void world() {
	float color[] = {0, 0, -4};
	float viewplane_dis = 1.0;
	float viewplane_scale = 1;
	data_to_uniform(color, 3, 1, "camera");
	data_to_uniform(&viewplane_dis, 1, 1, "viewplane_dis");
	data_to_uniform(&viewplane_scale, 1, 1, "viewplane_scale");
}
