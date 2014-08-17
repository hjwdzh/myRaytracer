#version 120

#define SAMPLE_SIZE 16
#define LIGHT_SIZE 64
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

//object
/* kd, ks, k3, x1, x2, x3, r1, r2, r3, s1, s2, s3, num
uniform sampler1D materialSampler;
uniform int num_object;

//Lights
uniform vec3 point_lights[LIGHT_SIZE];
uniform vec3 point_lights_color[LIGHT_SIZE];
uniform vec3 direct_lights[LIGHT_SIZE];
uniform vec3 direct_lights_color[LIGHT_SIZE];
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

int shadow_ray(vec3 ray_o_o, vec3 ray_t_o) {
	float step = 1 / (9.0 * num_triangles);
	float i = step / 2;
	int j = 0;
	for (int k = 0; k < num_object; ++k) {
	  float next_object = texture1D(materialSampler,(k+12.5/13)/(num_object)).r;
	  while (j < next_object) {
		float x1 = texture1D(materialSampler,(k+3.5/13)/(num_object)).r;
		float x2 = texture1D(materialSampler,(k+4.5/13)/(num_object)).r;
		float x3 = texture1D(materialSampler,(k+5.5/13)/(num_object)).r;
		vec3 a = vec3(texture1D(materialSampler,(k+6.5/13)/(num_object)).r,
		  				texture1D(materialSampler,(k+7.5/13)/(num_object)).r,
		  				texture1D(materialSampler,(k+8.5/13)/(num_object)).r);
		vec3 b = vec3(texture1D(materialSampler,(k+9.5/13)/(num_object)).r,
						texture1D(materialSampler,(k+10.5/13)/(num_object)).r,
		  				texture1D(materialSampler,(k+11.5/13)/(num_object)).r);
		vec3 c = cross(b,a);

		mat4 translate = mat4(vec4(1,0,0,0),vec4(0,1,0,0),vec4(0,0,1,0),
			vec4(x1,x2,x3,1));
		mat4 convert = inverse(translate * mat4(vec4(a,0),vec4(b,0),vec4(c,0),vec4(0,0,0,1)));
		vec3 ray_o = (convert * vec4(ray_o_o,1)).xyz;
		vec3 ray_t = (convert * vec4(ray_t_o,0)).xyz;

		vec3 v1 = vec3(texture1D(meshSampler,i).r, texture1D(meshSampler,i+step).r, texture1D(meshSampler,i+2*step).r);
		vec3 v2 = vec3(texture1D(meshSampler,i+3*step).r, texture1D(meshSampler,i+4*step).r, texture1D(meshSampler,i+5*step).r);
		vec3 v3 = vec3(texture1D(meshSampler,i+6*step).r, texture1D(meshSampler,i+7*step).r, texture1D(meshSampler,i+8*step).r);
		float u, v;
		float t = rayIntersectsTriangle(ray_o, ray_t, v1, v2, v3, u, v);
		if (t > 0)
			return 1;
		i += step * 9;
		j += 1;
	  }
	}
	return 0;
}

float tracing(vec3 ray_o_o, vec3 ray_t_o, inout int tri, inout int obj, inout vec3 normal) {
	float depth = 1e30;
	int j = 0;
	float step = 1 / (9.0 * num_triangles);
	float i = step / 2;
	obj = -1;
	tri = -1;
	vec3 normals;
	for (int k = 0; k < num_object; ++k) {
	  float x1 = texture1D(materialSampler,(k+3.5/13)/(num_object)).r;
	  float x2 = texture1D(materialSampler,(k+4.5/13)/(num_object)).r;
	  float x3 = texture1D(materialSampler,(k+5.5/13)/(num_object)).r;
	  vec3 a = vec3(texture1D(materialSampler,(k+6.5/13)/(num_object)).r,
	  				texture1D(materialSampler,(k+7.5/13)/(num_object)).r,
	  				texture1D(materialSampler,(k+8.5/13)/(num_object)).r);
	  vec3 b = vec3(texture1D(materialSampler,(k+9.5/13)/(num_object)).r,
	  				texture1D(materialSampler,(k+10.5/13)/(num_object)).r,
	  				texture1D(materialSampler,(k+11.5/13)/(num_object)).r);
	  vec3 c = cross(b,a);

	  mat4 translate = mat4(vec4(1,0,0,0),vec4(0,1,0,0),vec4(0,0,1,0),
	  	vec4(x1,x2,x3,1));
	  mat4 convert = inverse(translate * mat4(vec4(a,0),vec4(b,0),vec4(c,0),vec4(0,0,0,1)));
	  vec3 ray_o = (convert * vec4(ray_o_o,1)).xyz;
	  vec3 ray_t = (convert * vec4(ray_t_o,0)).xyz;

	  float next_object = texture1D(materialSampler,(k+12.5/13)/(num_object)).r;
	  while (j < next_object) {
		vec3 v1 = vec3(texture1D(meshSampler,i).r, texture1D(meshSampler,i+step).r, texture1D(meshSampler,i+2*step).r);
		vec3 v2 = vec3(texture1D(meshSampler,i+3*step).r, texture1D(meshSampler,i+4*step).r, texture1D(meshSampler,i+5*step).r);
		vec3 v3 = vec3(texture1D(meshSampler,i+6*step).r, texture1D(meshSampler,i+7*step).r, texture1D(meshSampler,i+8*step).r);
		float u, v;
		float t = rayIntersectsTriangle(ray_o, ray_t, v1, v2, v3, u, v);
		if (t > 0 && t < depth) {
			depth = t;
			obj = k;
			tri = j;
			vec3 n1 = vec3(texture1D(normalSampler,i).r, texture1D(normalSampler,i+step).r, texture1D(normalSampler,i+2*step).r);
			vec3 n2 = vec3(texture1D(normalSampler,i+3*step).r, texture1D(normalSampler,i+4*step).r, texture1D(normalSampler,i+5*step).r);
			vec3 n3 = vec3(texture1D(normalSampler,i+6*step).r, texture1D(normalSampler,i+7*step).r, texture1D(normalSampler,i+8*step).r);
			normal = u * (n2 - n1) + v * (n3 - n1) + n1;
		}
		i += step * 9;
		j += 1;
	  }
	}
	return depth;
}

vec3 lighting(vec3 point, vec3 normal, int tri_index, int obj_index) {
	float kd = texture1D(materialSampler, (obj_index+0.5/13)/num_object).r;
	float ks = texture1D(materialSampler, (obj_index+1.5/13)/num_object).r;
	float k3 = texture1D(materialSampler, (obj_index+2.5/13)/num_object).r;
	vec3 color = vec3(1,1,1) * ambient;
	vec3 eye_dir = normalize(camera - point);
	if (dot(eye_dir,normal) < 0)
		normal = -normal;
	for (int i = 0; i < num_direct_light; ++i) {
		float intensity = dot(-direct_lights[i], normal);
		if (intensity < 0)
			continue;
		if (shadow_ray(point, -direct_lights[i]) == 1)
			continue;
		color += intensity * vec3(1,1,1) * direct_lights_color[i]*kd 
			+ clamp(pow(dot(reflect(direct_lights[i], normal),eye_dir),20),0,1) * ks * direct_lights_color[i];
	}
	for (int i = 0; i < num_point_light; ++i) {
		vec3 dis = point - point_lights[i];
		float l = 1 / (length(dis));
		l = l * l;
		dis = normalize(dis);
		float intensity = dot(-dis, normal);
		if (intensity < 0)
			continue;
		if (shadow_ray(point, -dis) == 1)
			continue;
		vec3 para = kd * l * point_lights_color[i];
		color = color + vec3(1,1,1) * clamp(dot(-dis, normal), 0, 1) * para
			+ clamp(pow(dot(reflect(dis, normal),eye_dir),20),0,1) * ks * point_lights_color[i];
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
		int tri_index, obj_index;
		vec2 p = pixel * viewplane_scale + samples[i] / (480.0 * viewplane_scale);
		vec3 ray_t = normalize(viewplane_dis * camera_lookat+p.x*camera_x+p.y*camera_up);
		float depth = tracing(ray_o, ray_t, tri_index, obj_index, normal);
		if (depth < 1e20) {
			vec3 hit_point = ray_o + depth * ray_t;
			color += lighting(hit_point, normal, tri_index, obj_index);
		}
	}
	gl_FragColor.rgb = color / SAMPLE_SIZE;
	gl_FragColor.a = 1;
}
