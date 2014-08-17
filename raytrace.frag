#version 120

#define SAMPLE_SIZE 16
//Samplers
uniform vec2 samples[SAMPLE_SIZE];

//Camera parameters
uniform vec3 camera;
uniform vec3 camera_up;
uniform vec3 camera_lookat;

//Viewplane parameters
uniform float viewplane_dis;
uniform vec2 viewplane_scale;

//Mesh
uniform sampler1D meshSampler;
uniform sampler1D normalSampler;
uniform int num_triangles;

//Lights
uniform vec3 point_lights[64];
uniform vec3 point_lights_color[64];
uniform vec3 direct_lights[64];
uniform vec3 direct_lights_color[64];
uniform int num_point_light, num_direct_light;
uniform vec3 ambient;

//Current viewplane coord
varying vec2 pixel;

float rayIntersectsTriangle(vec3 p, vec3 d,
			vec3 v0, vec3 v1, vec3 v2, inout float u, inout float v) {

	vec3 e1 = v1 - v0;
	vec3 e2 = v2 - v0;
	vec3 h = cross(d,e2);
	float a = dot(e1, h);

	if (a > -0.00001 && a < 0.00001)
		return 0;

	float f = 1/a;
	vec3 s = p - v0;
	u = f * dot(s,h);

	if (u < 0.0 || u > 1.0)
		return 0;

	vec3 q = cross(s,e1);
	v = f * dot(d,q);

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

float hit(vec3 ray_o, vec3 ray_t, inout int obj, inout vec3 normal) {

	float depth = 1e30;
	int j = 0;
	float step = 1 / (9.0 * num_triangles);
	float i = step / 2;
	obj = -1;
	vec3 normals;
	for (int j = 1; j <= num_triangles; ++j) {
		vec3 v1 = vec3(texture1D(meshSampler,i).r, texture1D(meshSampler,i+step).r, texture1D(meshSampler,i+2*step).r);
		vec3 v2 = vec3(texture1D(meshSampler,i+3*step).r, texture1D(meshSampler,i+4*step).r, texture1D(meshSampler,i+5*step).r);
		vec3 v3 = vec3(texture1D(meshSampler,i+6*step).r, texture1D(meshSampler,i+7*step).r, texture1D(meshSampler,i+8*step).r);
		float u, v;
		float t = rayIntersectsTriangle(ray_o, ray_t, v1, v2, v3, u, v);
		if (t > 0 && t < depth) {
			depth = t;
			obj = j;
			vec3 n1 = vec3(texture1D(normalSampler,i).r, texture1D(normalSampler,i+step).r, texture1D(normalSampler,i+2*step).r);
			vec3 n2 = vec3(texture1D(normalSampler,i+3*step).r, texture1D(normalSampler,i+4*step).r, texture1D(normalSampler,i+5*step).r);
			vec3 n3 = vec3(texture1D(normalSampler,i+6*step).r, texture1D(normalSampler,i+7*step).r, texture1D(normalSampler,i+8*step).r);
			normal = u * (n2 - n1) + v * (n3 - n1) + n1;
		}
		i += step * 9;
	}
	return depth;
}

vec3 tracing(vec3 point, vec3 normal, int index) {
	vec3 color = vec3(1,1,1) * ambient;
	for (int i = 0; i < num_direct_light; ++i) {
		float intensity = dot(-direct_lights[i], normal);
		color = color + intensity * vec3(1,1,1) * direct_lights_color[i];
	}
	for (int i = 0; i < num_point_light; ++i) {
		vec3 dis = point_lights[i] - point;
		float l = 1 / (length(dis));
		l = l * l;
		vec3 para = l * point_lights_color[i];
		color = color + vec3(1,1,1) * clamp(
			dot(normalize(dis), normal), 0, 1)
			* para;
	}
	return color;
}

void main() {
	// Output color = red 
	vec3 color = vec3(0,0,0);
	vec3 ray_o = camera;
	vec3 camera_x = cross(camera_lookat, camera_up);
	vec3 normal = vec3(0,0,0);
	for (int i = 0; i < SAMPLE_SIZE; ++i) {
		int index;
		vec2 p = pixel * viewplane_scale + samples[i] / (480.0 * viewplane_scale);
		vec3 ray_t = normalize(viewplane_dis * camera_lookat+p.x*camera_x+p.y*camera_up);
		float depth = hit(ray_o, ray_t, index, normal);
		if (depth < 1e20) {
			vec3 hit_point = ray_o + depth * ray_t;
			color += tracing(hit_point, normal, index);
		}
	}
	gl_FragColor.rgb = color / SAMPLE_SIZE;
	gl_FragColor.a = 1;
}