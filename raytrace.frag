#version 120

#define SAMPLE_SIZE 16

uniform vec2 samples[SAMPLE_SIZE];

uniform vec3 camera;
uniform float viewplane_dis;
uniform vec2 viewplane_scale;

uniform sampler1D meshSampler;
uniform int num_triangles;

varying vec2 pixel;

float rayIntersectsTriangle(vec3 p, vec3 d,
			vec3 v0, vec3 v1, vec3 v2) {

	vec3 e1 = v1 - v0;
	vec3 e2 = v2 - v0;
	vec3 h = cross(d,e2);
	float a = dot(e1, h);

	if (a > -0.00001 && a < 0.00001)
		return 0;

	float f = 1/a;
	vec3 s = p - v0;
	float u = f * dot(s,h);

	if (u < 0.0 || u > 1.0)
		return 0;

	vec3 q = cross(s,e1);
	float v = f * dot(d,q);

	if (v < 0.0 || u + v > 1.0)
		return 0;

	// at this stage we can compute t to find out where
	// the intersection point is on the line
	float t = f * dot(e2,q);

	if (t > 0.00001) // ray intersection
		return t;

	else // this means that there is a line intersection
		 // but not a ray intersection
		return 0;
}

vec4 hit(vec3 ray_o, vec3 ray_t) {

	float depth = 1e30;
	float object = -1;
	int j = 0;
	float step = 1 / (9.0 * num_triangles);
	float i = step / 2;
	for (int j = 1; j <= num_triangles; ++j) {
		vec3 v1 = vec3(texture1D(meshSampler,i).r, texture1D(meshSampler,i+step).r, texture1D(meshSampler,i+2*step).r);
		vec3 v2 = vec3(texture1D(meshSampler,i+3*step).r, texture1D(meshSampler,i+4*step).r, texture1D(meshSampler,i+5*step).r);
		vec3 v3 = vec3(texture1D(meshSampler,i+6*step).r, texture1D(meshSampler,i+7*step).r, texture1D(meshSampler,i+8*step).r);
		float t = rayIntersectsTriangle(ray_o, ray_t, v1, v2, v3);
		i += step * 9;
		if (t > 0 && t < depth) {
			depth = t;
			object = (j+0.0)/num_triangles;
		}
	}
	return vec4(object,0,0,1);
}

void main() {
	// Output color = red 
	vec4 color = vec4(0,0,0,0);
	vec3 ray_o = camera;
//	int i = 0;
	for (int i = 0; i < SAMPLE_SIZE; ++i) {
		vec2 p = pixel * viewplane_scale + samples[i] / (480.0 * viewplane_scale);
		vec3 ray_t = normalize(viewplane_dis * vec3(0,0,1)+vec3(p.x,p.y,0));
		color = color + hit(ray_o, ray_t);
	}
	gl_FragColor = color / SAMPLE_SIZE;
//	gl_FragColor = vec4(0,0,0,1);
//	gl_FragColor.r = texture1D(meshSampler,(pixel.x+1)/2).r;
}