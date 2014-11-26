#version 120

#define SAMPLE_SIZE 64
#define LIGHT_SIZE 64
#define TEXTURE_SIZE 9
#define MATERIAL_LEN 21
#define BVH_LEN 100
#define PATH_DEPTH 5

//Samplers

//Camera parameters
uniform vec3 camera;
uniform vec3 camera_up;
uniform vec3 camera_lookat;

//Viewplane parameters
uniform float viewplane_dis;
uniform vec2 viewplane_scale;

//Texture
uniform sampler2D renderSampler[TEXTURE_SIZE];

//Mesh
uniform sampler2D meshSampler;
uniform sampler2D normalSampler;
uniform sampler1D samples;
// kd, ks, tex, offset(3), rotate(3), scale(3), next_object
uniform sampler2D materialSampler;
uniform sampler2D texSampler;
uniform sampler1D indexSampler;
// mincorner(3), maxcorner(3), axis, v-index, l-index, r-index, p-index
uniform sampler2D bvhSampler;
uniform int num_triangles;
uniform int num_object;
uniform int num_bvh;
uniform int use_bvh;
uniform int use_path;

//Lights
uniform vec3 point_lights[LIGHT_SIZE];
uniform vec3 point_lights_color[LIGHT_SIZE];
uniform vec3 direct_lights[LIGHT_SIZE];
uniform vec3 direct_lights_color[LIGHT_SIZE];
uniform int num_point_light, num_direct_light;
uniform vec3 ambient;


//Current viewplane coord
varying vec2 pixel;

// Intersection Test
float rayIntersectsTriangle(vec3 p, vec3 d,
			vec3 v0, vec3 v1, vec3 v2, inout float u, inout float v) {

	vec3 e1 = v1 - v0;
	vec3 e2 = v2 - v0;
	vec3 h = cross(d,e2);
	float a = dot(e1, h);

	if (a > -0.00001 && a < 0.00001)
		return -1;

	float f = 1/a;
	vec3 s = p - v0;
	u = f * dot(s,h);

	if (u < 0.0 || u > 1.0)
		return -1;

	vec3 q = cross(s,e1);
	v = f * dot(d,q);

	if (v < 0.0 || u + v > 1.0)
		return -1;

	// at this stage we can compute t to find out where
	// the intersection point is on the line
	float t = f * dot(e2,q);

	if (t > 0.00001) // ray intersection
		return t;

	else // this means that there is a line intersection
		 // but not a ray intersection
		return -1;
}

float box_intersect(float bvh_ind, vec3 ray_o, vec3 ray_t) {
	float step = 1.0 / 11;
	float tmp = 0;
	float a1, a2, b1, b2, c1, c2;
	a1 = (texture2D(bvhSampler, vec2(0.5 * step, (bvh_ind+0.5) / num_bvh)).r - ray_o.x);
	a2 = (texture2D(bvhSampler, vec2(3.5 * step, (bvh_ind+0.5) / num_bvh)).r - ray_o.x);
	if (ray_t.x < 1e-6 && ray_t.x > -1e-6) {
		if (a1 * a2 > 1e-4)
			return -1;
		a1 = -1e30; a2 = 1e30;
	} else {
		a1 /= ray_t.x;
		a2 /= ray_t.x;
	}
	if (a1 > a2) {
		tmp = a1; a1 = a2; a2 = tmp;
	}
	b1 = (texture2D(bvhSampler, vec2(1.5 * step, (bvh_ind+0.5) / num_bvh)).r - ray_o.y);
	b2 = (texture2D(bvhSampler, vec2(4.5 * step, (bvh_ind+0.5) / num_bvh)).r - ray_o.y);
	if (ray_t.y < 1e-6 && ray_t.y > -1e-6) {
		if (b1 * b2 > 1e-4)
			return -1;
		b1 = -1e30; b2 = 1e30;
	} else {
		b1 /= ray_t.y;
		b2 /= ray_t.y;
	}
	if (b1 > b2) {
		tmp = b1; b1 = b2; b2 = tmp;
	}
	c1 = (texture2D(bvhSampler, vec2(2.5 * step, (bvh_ind+0.5) / num_bvh)).r - ray_o.z);
	c2 = (texture2D(bvhSampler, vec2(5.5 * step, (bvh_ind+0.5) / num_bvh)).r - ray_o.z);
	if (ray_t.z < 1e-6 && ray_t.z > -1e-6) {
		if (c1 * c2 > 1e-4)
			return -1;
		c1 = -1e30; c2 = 1e30;
	} else {
		c1 /= ray_t.z;
		c2 /= ray_t.z;
	}
	if (c1 > c2) {
		tmp = c1; c1 = c2; c2 = tmp;
	}
	float t1, t2;
	t1 = max(a1, max(b1, c1));
	t2 = min(a2, min(b2, c2));

	if (t2 >= t1 && t2 >= 0)
		return max(t1, 0);
	else
		return -1;
}

float bvh_index(float bvh_node) {
	return texture2D(bvhSampler, vec2(7.5 / 11, (bvh_node + 0.5) / num_bvh)).r;
}

float bvh_left(float bvh_node) {
	return texture2D(bvhSampler, vec2(8.5 / 11, (bvh_node + 0.5) / num_bvh)).r;
}

float bvh_right(float bvh_node) {
	return texture2D(bvhSampler, vec2(9.5 / 11, (bvh_node + 0.5) / num_bvh)).r;
}

float bvh_parent(float bvh_node) {
	return texture2D(bvhSampler, vec2(10.5 / 11,(bvh_node + 0.5) / num_bvh)).r;
}

bool bvh_dir(float bvh_node, vec3 ray) {
	float axis = texture2D(bvhSampler, vec2(6.5 / 11, (bvh_node + 0.5) / num_bvh)).r;
	if (axis < 0.5)
		return ray.x > 0;
	if (axis < 1.5)
		return ray.y > 0;
	return ray.z <= 0;
}

float tri_intersect(float v_node, vec3 ray_o, vec3 ray_t, inout float u, inout float v) {
	float step = 1 / 9.0;
	v_node = (v_node + 0.5) / num_triangles;
	vec3 v1 = vec3(texture2D(meshSampler,vec2(0.5*step,v_node)).r, texture2D(meshSampler,vec2(1.5*step,v_node)).r, texture2D(meshSampler,vec2(2.5*step,v_node)).r);
	vec3 v2 = vec3(texture2D(meshSampler,vec2(3.5*step,v_node)).r, texture2D(meshSampler,vec2(4.5*step,v_node)).r, texture2D(meshSampler,vec2(5.5*step,v_node)).r);
	vec3 v3 = vec3(texture2D(meshSampler,vec2(6.5*step,v_node)).r, texture2D(meshSampler,vec2(7.5*step,v_node)).r, texture2D(meshSampler,vec2(8.5*step,v_node)).r);
	return rayIntersectsTriangle(ray_o, ray_t, v1, v2, v3, u, v);
}

float bvh_intersect(vec3 ray_o, vec3 ray_t, inout float index, inout float u, inout float v) {
	float depth = 1e30;
	index = -1;
	float bvh_node = 0;
	float last_node = -1;
	float u1, v1;
	int t = 0;
	while (bvh_node >= 0) {
		t += 1;
		if (last_node == -1) {
			float cur_depth = box_intersect(bvh_node, ray_o, ray_t);
			if (cur_depth < 0 || cur_depth > depth) {
				last_node = bvh_node;
				bvh_node = bvh_parent(bvh_node);
				continue;
			}
			if (bvh_left(bvh_node) < 0) {
				float cur_index = bvh_index(bvh_node);
				cur_depth = tri_intersect(cur_index, ray_o, ray_t, u1, v1);
				if (cur_depth >= 0 && cur_depth < depth) {
					index = cur_index;
					u = u1;
					v = v1;
					depth = cur_depth;
				}
				last_node = bvh_node;
				bvh_node = bvh_parent(bvh_node);
				continue;
			} else {
				last_node = -1;
				if (bvh_dir(bvh_node, ray_t)) {
					bvh_node = bvh_left(bvh_node);
				} else {
					bvh_node = bvh_right(bvh_node);
				}
			}
		} else {
			bool dir = bvh_dir(bvh_node, ray_t);
			float left_node = bvh_left(bvh_node);
			float right_node = bvh_right(bvh_node);
			if (dir && left_node == last_node) {
				last_node = -1;
				bvh_node = bvh_right(bvh_node);
			} else
			if (!dir && right_node == last_node) {
				last_node = -1;
				bvh_node = bvh_left(bvh_node);
			} else {
				last_node = bvh_node;
				bvh_node = bvh_parent(bvh_node);
			}
		}
	}
	return depth;
}

float bvh_tracing(vec3 ray_o, vec3 ray_t, inout float tri, inout float obj, inout vec3 hit_point, inout vec2 uv, inout vec3 normal) {
	float u, v, tri_ind;
	float depth = 1e30;
	depth = bvh_intersect(ray_o, ray_t, tri_ind, u, v);
	if (depth > 1e20)
		return depth;
	tri = tri_ind;
	obj = texture1D(indexSampler, (tri_ind + 0.5) / num_triangles).r;
	hit_point = ray_o + depth * ray_t;
	float i = (tri_ind + 0.5) / num_triangles;
	float step = 1 / 9.0 ;
	vec3 n1 = vec3(texture2D(normalSampler,vec2(0.5*step,i)).r, texture2D(normalSampler,vec2(1.5*step,i)).r, texture2D(normalSampler,vec2(2.5*step,i)).r);
	vec3 n2 = vec3(texture2D(normalSampler,vec2(3.5*step,i)).r, texture2D(normalSampler,vec2(4.5*step,i)).r, texture2D(normalSampler,vec2(5.5*step,i)).r);
	vec3 n3 = vec3(texture2D(normalSampler,vec2(6.5*step,i)).r, texture2D(normalSampler,vec2(7.5*step,i)).r, texture2D(normalSampler,vec2(8.5*step,i)).r);
	normal = normalize(u * (n2 - n1) + v * (n3 - n1) + n1);
	float tex_step = 1 / 6.0;
	hit_point = ray_o + depth * ray_t;
	vec2 uv1 = vec2(texture2D(texSampler, vec2(tex_step * 0.5, i)).r, texture2D(texSampler, vec2(tex_step * 1.5, i)).r);
	vec2 uv2 = vec2(texture2D(texSampler, vec2(tex_step * 2.5, i)).r, texture2D(texSampler, vec2(tex_step * 3.5, i)).r);
	vec2 uv3 = vec2(texture2D(texSampler, vec2(tex_step * 4.5, i)).r, texture2D(texSampler, vec2(tex_step * 5.5, i)).r);
	uv = uv1 + u * (uv2 - uv1) + v * (uv3 - uv1);	
	return depth;
}

vec3 bvh_lighting(vec3 start_camera, vec3 point, vec3 normal, float tri_index, vec2 uv, float obj_index, inout vec3 orig_color) {
	float kd = texture2D(materialSampler, vec2(0.5/MATERIAL_LEN,(obj_index+0.5)/num_object)).r;
	float ks = texture2D(materialSampler, vec2(1.5/MATERIAL_LEN,(obj_index+0.5)/num_object)).r;
	float ka = texture2D(materialSampler, vec2(16.5/MATERIAL_LEN,(obj_index+0.5)/num_object)).r;
	float alpha = texture2D(materialSampler, vec2(20.5/MATERIAL_LEN,(obj_index+0.5)/num_object)).r;
	int tex = int(0.1+texture2D(materialSampler, vec2(2.5/MATERIAL_LEN,(obj_index+0.5)/num_object)).r);
	orig_color = texture2D(renderSampler[tex], uv).rgb;
	vec3 color = ka * orig_color * ambient;
	vec3 eye_dir = normalize(start_camera - point);
	float t1,t2,t3;
	for (int i = 0; i < num_direct_light; ++i) {
		float intensity = dot(-direct_lights[i], normal) * dot(eye_dir, normal);
		if (intensity < 0)
			continue;
		float dep = bvh_intersect(point, -direct_lights[i], t1,t2,t3);
		if (dep < 1000)
			continue;
		color += intensity * (orig_color * direct_lights_color[i]*kd
			+ clamp(pow(dot(reflect(direct_lights[i], normal),eye_dir),20),0,1) * ks * direct_lights_color[i]);
	}
	for (int i = 0; i < num_point_light; ++i) {
		vec3 dis = point - point_lights[i];
		float l = 1 / (length(dis));
		l = l * l;
		dis = normalize(dis);
		float intensity = dot(-dis, normal) * dot(eye_dir, normal);
		if (intensity < 0)
			continue;
		float dep = bvh_intersect(point, -dis,t1,t2,t3);
		if (dep < length(dis))
			continue;
		vec3 para = kd * l * point_lights_color[i];
		color = color + intensity * (orig_color * para
			+ clamp(pow(dot(reflect(dis, normal),eye_dir),alpha),0,1) * ks * point_lights_color[i]);
	}
	return color;
}

float tracing(vec3 ray_o_o, vec3 ray_t_o, float shadow, inout float tri, inout float obj, inout vec3 hit_point, inout vec2 uv, inout vec3 normal) {
	float depth = 1e30;
	int j = 0;
	float step = 1 / 9.0;
	float vstep = 1.0 / num_triangles;
	float tex_step = 1 / 6.0;
	float i = vstep / 2;
	obj = -1;
	tri = -1;
	vec3 normals;
	for (int k = 0; k < num_object; ++k) {
	  float x1 = texture2D(materialSampler,vec2(3.5/MATERIAL_LEN,(k+0.5)/num_object)).r;
	  float x2 = texture2D(materialSampler,vec2(4.5/MATERIAL_LEN,(k+0.5)/num_object)).r;
	  float x3 = texture2D(materialSampler,vec2(5.5/MATERIAL_LEN,(k+0.5)/num_object)).r;
	  vec3 a = vec3(texture2D(materialSampler,vec2(6.5/MATERIAL_LEN,(k+0.5)/num_object)).r,
		  				texture2D(materialSampler,vec2(7.5/MATERIAL_LEN,(k+0.5)/num_object)).r,
		  				texture2D(materialSampler,vec2(8.5/MATERIAL_LEN,(k+0.5)/num_object)).r);
	  vec3 b = vec3(texture2D(materialSampler,vec2(9.5/MATERIAL_LEN,(k+0.5)/num_object)).r,
						texture2D(materialSampler,vec2(10.5/MATERIAL_LEN,(k+0.5)/num_object)).r,
		  				texture2D(materialSampler,vec2(11.5/MATERIAL_LEN,(k+0.5)/num_object)).r);
	  vec3 c = cross(a,b);

	  vec3 sv = vec3(texture2D(materialSampler,vec2(12.5/MATERIAL_LEN,(k+0.5)/num_object)).r,
						texture2D(materialSampler,vec2(13.5/MATERIAL_LEN,(k+0.5)/num_object)).r,
		  				texture2D(materialSampler,vec2(14.5/MATERIAL_LEN,(k+0.5)/num_object)).r);

	  mat4 rotate = mat4(vec4(a,0),vec4(b,0),vec4(c,0),vec4(0,0,0,1));
	  mat4 convert = mat4(vec4(1,0,0,0),vec4(0,1,0,0),vec4(0,0,1,0), vec4(x1,x2,x3,1)) 
	  			   * rotate 
	  			   * mat4(vec4(sv.x,0,0,0),vec4(0,sv.y,0,0),vec4(0,0,sv.z,0),vec4(0,0,0,1));
	  mat4 inv_convert = mat4(vec4(1/sv.x,0,0,0),vec4(0,1/sv.y,0,0),vec4(0,0,1/sv.z,0),vec4(0,0,0,1))
	  				   * transpose(rotate) 
	  				   * mat4(vec4(1,0,0,0),vec4(0,1,0,0),vec4(0,0,1,0),vec4(-x1,-x2,-x3,1));

  	  vec3 ray_o = (inv_convert * vec4(ray_o_o,1)).xyz;
      vec3 ray_t = (inv_convert * vec4(ray_t_o,0)).xyz;	  

	  float next_object = texture2D(materialSampler,vec2(15.5/MATERIAL_LEN,(k+0.5)/num_object)).r;
	  while (j < next_object) {
		vec3 v1 = vec3(texture2D(meshSampler,vec2(0.5*step,i)).r, texture2D(meshSampler,vec2(1.5*step,i)).r, texture2D(meshSampler,vec2(2.5*step,i)).r);
		vec3 v2 = vec3(texture2D(meshSampler,vec2(3.5*step,i)).r, texture2D(meshSampler,vec2(4.5*step,i)).r, texture2D(meshSampler,vec2(5.5*step,i)).r);
		vec3 v3 = vec3(texture2D(meshSampler,vec2(6.5*step,i)).r, texture2D(meshSampler,vec2(7.5*step,i)).r, texture2D(meshSampler,vec2(8.5*step,i)).r);
		float u, v;
		float t = rayIntersectsTriangle(ray_o, ray_t, v1, v2, v3, u, v);
		if (t > 0 && t < depth) {
			depth = t;
			hit_point = (convert * vec4(ray_o + ray_t * depth,1)).xyz;
			if (shadow >= 0) {
				if (t < shadow) {
					return t;
				}
			} else {
				obj = k;
				tri = j;
				vec3 n1 = vec3(texture2D(normalSampler,vec2(0.5*step,i)).r, texture2D(normalSampler,vec2(1.5*step,i)).r, texture2D(normalSampler,vec2(2.5*step,i)).r);
				vec3 n2 = vec3(texture2D(normalSampler,vec2(3.5*step,i)).r, texture2D(normalSampler,vec2(4.5*step,i)).r, texture2D(normalSampler,vec2(5.5*step,i)).r);
				vec3 n3 = vec3(texture2D(normalSampler,vec2(6.5*step,i)).r, texture2D(normalSampler,vec2(7.5*step,i)).r, texture2D(normalSampler,vec2(8.5*step,i)).r);
				normal = (convert * vec4(u * (n2 - n1) + v * (n3 - n1) + n1,0)).xyz;
				vec2 uv1 = vec2(texture2D(texSampler, vec2(tex_step * 0.5, i)).r, texture2D(texSampler, vec2(tex_step * 1.5, i)).r);
				vec2 uv2 = vec2(texture2D(texSampler, vec2(tex_step * 2.5, i)).r, texture2D(texSampler, vec2(tex_step * 3.5, i)).r);
				vec2 uv3 = vec2(texture2D(texSampler, vec2(tex_step * 4.5, i)).r, texture2D(texSampler, vec2(tex_step * 5.5, i)).r);
				uv = uv1 + u * (uv2 - uv1) + v * (uv3 - uv1);
			}
		}
		i += vstep;
		j += 3;
	  }
	}
	normal = normalize(normal);
	return depth;
}

vec3 lighting(vec3 start_camera, vec3 point, vec3 normal, float tri_index, vec2 uv, float obj_index, inout vec3 orig_color) {
	float kd = texture2D(materialSampler, vec2(0.5/MATERIAL_LEN,(obj_index+0.5)/num_object)).r;
	float ks = texture2D(materialSampler, vec2(1.5/MATERIAL_LEN,(obj_index+0.5)/num_object)).r;
	float ka = texture2D(materialSampler, vec2(16.5/MATERIAL_LEN,(obj_index+0.5)/num_object)).r;
	float alpha = texture2D(materialSampler, vec2(20.5/MATERIAL_LEN,(obj_index+0.5)/num_object)).r;
	int tex = int(0.1+texture2D(materialSampler, vec2(2.5/MATERIAL_LEN,(obj_index+0.5)/num_object)).r);
	orig_color = texture2D(renderSampler[tex], uv).rgb;
	vec3 color = ka * orig_color * ambient;
	vec3 eye_dir = normalize(start_camera - point);
	float t1, t2;
	vec2 v1;
	vec3 v2,v3;
	for (int i = 0; i < num_direct_light; ++i) {
		float intensity = dot(-direct_lights[i], normal) * dot(eye_dir, normal);
		if (intensity < 0)
			continue;
		float depth = tracing(point, -direct_lights[i], 100, t1, t2, v2,v1, v3);
		if (depth < 1000)
			continue;
		color += intensity * (orig_color * direct_lights_color[i]*kd
			+ clamp(pow(dot(reflect(direct_lights[i], normal),eye_dir),20),0,1) * ks * direct_lights_color[i]);
	}
	for (int i = 0; i < num_point_light; ++i) {
		vec3 dis = point - point_lights[i];
		float len = length(dis);
		float l = 1 / (len * len);
		dis = normalize(dis);
		float intensity = dot(-dis, normal) * dot(eye_dir, normal);
		if (intensity < 0)
			continue;
		float depth = tracing(point, -dis, len, t1, t2, v2,v1, v3);
		if (depth < len)
			continue;
		vec3 para = kd * l * point_lights_color[i];
		color = color + intensity * (orig_color * para
			+ clamp(pow(dot(reflect(dis, normal),eye_dir),alpha),0,1) * ks * point_lights_color[i]);
	}
	return color;
}

float extract_reflection(float obj_index) {
	return texture2D(materialSampler, vec2(17.5/MATERIAL_LEN,(obj_index+0.5)/num_object)).r;	
}

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 phoneSample(vec3 ray_t, vec3 normal, float alpha) {
	float sample_id = texture1D(samples, (SAMPLE_SIZE*2+0.5)/(SAMPLE_SIZE*2+2)).r;
	vec2 noises;
	if (sample_id < 0) {
		noises.x = texture1D(samples, (0.5) / (SAMPLE_SIZE*2+2)).r;
		noises.y = texture1D(samples, (1.5) / (SAMPLE_SIZE*2+2)).r;
	} else {
		noises.x = texture1D(samples, (0.5 + 2 * sample_id) / (SAMPLE_SIZE*2+2)).r;
		noises.y = texture1D(samples, (1.5 + 2 * sample_id) / (SAMPLE_SIZE*2+2)).r;	
	}
	bool terminate = false;
	vec3 n = ray_t - 2 * dot(ray_t, normal) * normal;
	float theta = acos(sqrt(1 - noises.x));
	float phi = 2.0 * 3.1415926535897932384626433832795 * noises.y;
	float xs = sin(theta) * pow(cos(phi), alpha);
	float ys = cos(theta);
	float zs = sqrt(1 - xs * xs - ys * ys);
	vec3 h = vec3(4.4857, 8.58343, 2.4375);
	vec3 x = normalize(cross(h,n));
	vec3 z = normalize(cross(x,n));
	vec3 d = xs * x + ys * n + zs * z;
	if (dot(d,normal) < 0) {
		d = n;
	}
	return normalize(d);
}

void main() {
	vec3 color = vec3(0,0,0);
	vec3 ray_o = camera;
	vec3 camera_x = cross(camera_lookat, camera_up);
	vec3 normal = vec3(0,0,0);
	vec3 hit_point = vec3(0,0,0);
	vec2 uv;
	float sample_id = texture1D(samples, (SAMPLE_SIZE*2+0.5)/(SAMPLE_SIZE*2+2)).r;
	int path_state[PATH_DEPTH];
	float mat_stack[PATH_DEPTH];
	vec3 light_stack[PATH_DEPTH];
	vec3 color_stack[PATH_DEPTH];
	vec3 from_stack[PATH_DEPTH];
	vec3 to_stack[PATH_DEPTH];
	vec3 normal_stack[PATH_DEPTH];
	int node = 0;
	vec3 orig_color;
	if (sample_id < 0) {
		for (int i = 0; i < SAMPLE_SIZE; ++i) {
			float tri_index, obj_index;
			vec2 noise;
			noise.x = texture1D(samples, (2 * i + 0.5) / (SAMPLE_SIZE*2+2)).r;
			noise.y = texture1D(samples, (2 * i + 1.5) / (SAMPLE_SIZE*2+2)).r;
			vec2 p = pixel * viewplane_scale + noise / (480.0 * viewplane_scale);
			vec3 ray_t = normalize(viewplane_dis * camera_lookat+p.x*camera_x+p.y*camera_up);
			float depth = 0;
			if (use_bvh == 1)
				depth = bvh_tracing(ray_o, ray_t, tri_index, obj_index, hit_point, uv, normal);
			else
				depth = tracing(ray_o, ray_t, 0, tri_index, obj_index, hit_point, uv, normal);
			normal = vec3(0,0,-1);
			if (depth < 1e20) {
				if (use_bvh == 1)
					color += bvh_lighting(camera, hit_point, normal, tri_index, uv, obj_index, orig_color);
				else
					color += lighting(camera, hit_point, normal, tri_index, uv, obj_index, orig_color);
			}
		}
		gl_FragColor.rgb = color / SAMPLE_SIZE;
	} else {
		float tri_index, obj_index;

		vec2 noise;
		noise.x = texture1D(samples, (2 * sample_id + 0.5) / (SAMPLE_SIZE*2+2)).r;
		noise.y = texture1D(samples, (2 * sample_id + 1.5) / (SAMPLE_SIZE*2+2)).r;
		vec2 p = pixel * viewplane_scale + noise / (480.0 * viewplane_scale);
		vec3 ray_t = normalize(viewplane_dis * camera_lookat+p.x*camera_x+p.y*camera_up);
		path_state[node] = 0;
		from_stack[node] = ray_o;
		to_stack[node] = ray_t;
		color_stack[node] = vec3(0,0,0);
		light_stack[node] = vec3(0,0,0);
		float nr;
		int hit_mat = 0;
		while (node >= 0) {
			if (path_state[node] == 0) {
				path_state[node] = 1;
				float depth;
				if (use_bvh == 1)
					depth = bvh_tracing(from_stack[node], to_stack[node], tri_index, obj_index, hit_point, uv, normal);
				else
					depth = tracing(from_stack[node], to_stack[node], -1, tri_index, obj_index, hit_point, uv, normal);
				if (depth < 1e20) {
					if (use_bvh == 1)
						light_stack[node] = bvh_lighting(from_stack[node], hit_point, normal, tri_index, uv, obj_index, orig_color);
					else
						light_stack[node] = lighting(from_stack[node], hit_point, normal, tri_index, uv, obj_index, orig_color);
					color_stack[node] = orig_color;
					normal_stack[node] = normal;
					ray_t = to_stack[node];
					to_stack[node] = hit_point;
					mat_stack[node] = obj_index;
					float kr = extract_reflection(obj_index);
					if (kr > 0 && node < PATH_DEPTH - 1) {
						node += 1;
						path_state[node] = 0;
						from_stack[node] = hit_point;
						to_stack[node] = ray_t - 2 * dot(ray_t, normal) * normal;
						light_stack[node] = vec3(0,0,0);
						continue;
					}
				} else {
					path_state[node] = 3;
				}
			}
			if (path_state[node] == 1) {
				path_state[node] = 2;
				obj_index = mat_stack[node];
				float kf = texture2D(materialSampler, vec2(18.5/MATERIAL_LEN,(obj_index+0.5)/num_object)).r;
				if (kf > 0 && node < PATH_DEPTH - 1) {
					nr = texture2D(materialSampler, vec2(19.5/MATERIAL_LEN,(obj_index+0.5)/num_object)).r;
					normal = normal_stack[node];
					ray_t = normalize(to_stack[node] - from_stack[node]);
					float cost = dot(normal, ray_t);
					if (cost < 0) {
						nr = 1 / nr;
						cost = -cost;
					} else {
						normal = -normal;
					}
					float rootContent = 1 - nr * nr * (1 - cost * cost);
					if (rootContent >= 0) {
						rootContent = sqrt(rootContent);
						node += 1;
						path_state[node] = 0;
						from_stack[node] = to_stack[node - 1];
						to_stack[node] = (nr * cost - rootContent) * normal + nr * ray_t;
						light_stack[node] = vec3(0,0,0);
						continue;
					}
				}
			}
			if (path_state[node] == 2) {
				path_state[node] = 3;
				obj_index = mat_stack[node];
				float ks = texture2D(materialSampler, vec2(1.5/MATERIAL_LEN,(obj_index+0.5)/num_object)).r;
				if (hit_mat < use_path && node < PATH_DEPTH - 1 && ks > 0) {
					normal = normal_stack[node];
					ray_t = normalize(to_stack[node] - from_stack[node]);
					float alpha = texture2D(materialSampler, vec2(20.5/MATERIAL_LEN,(obj_index+0.5)/num_object)).r;
					hit_mat += 1;
					node += 1;
					path_state[node] = 0;
					from_stack[node] = to_stack[node - 1];
					to_stack[node] = phoneSample(ray_t, normal, alpha);
					continue;
				}
			}
			if (path_state[node] == 3) {
				if (node == 0)
					break;
				float obj_index = mat_stack[node - 1];
				if (path_state[node - 1] == 1) {
					light_stack[node - 1] += extract_reflection(obj_index) * color_stack[node - 1] * light_stack[node];
				}
				else
				if (path_state[node - 1] == 2) {
					light_stack[node - 1] += texture2D(materialSampler, vec2(18.5/MATERIAL_LEN,(obj_index+0.5)/num_object)).r * color_stack[node - 1] * light_stack[node];
				}
				else {
					hit_mat -= 1;
					normal = normal_stack[node - 1];
					ray_t = normalize(to_stack[node - 1] - from_stack[node - 1]);
					float alpha = texture2D(materialSampler, vec2(20.5/MATERIAL_LEN,(obj_index+0.5)/num_object)).r;
					light_stack[node - 1] += texture2D(materialSampler, vec2(1.5/MATERIAL_LEN,(obj_index+0.5)/num_object)).r * color_stack[node - 1] * light_stack[node] * dot(-ray_t, normal);
				}
				node -= 1;
			}
		}
		gl_FragColor.rgb = light_stack[0];
	}
	gl_FragColor.a = 1;
}