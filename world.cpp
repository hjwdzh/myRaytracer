#include "opengl_util.h"

//Set the render world
void world() {
	float color[] = {1.0, 0.5, 0.3};
	data_to_uniform(color, 3, 1, "color");
}
