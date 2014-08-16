#version 120

uniform vec3 camera;
uniform float viewplane_dis;
uniform float viewplane_scale;

uniform sampler1D meshSampler;
uniform float step;

varying vec2 pixel;

int rayIntersectsTriangle(vec3 p, vec3 d,
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
		return 1;

	else // this means that there is a line intersection
		 // but not a ray intersection
		return 0;
}

vec4 hit(vec3 ray_o, vec3 ray_t) {

	float i = step/2;
//	for (float i = 0; i < 1; i += 9 * step) {
		vec3 v1 = vec3(texture1D(meshSampler,i).r, texture1D(meshSampler,i+step).r, texture1D(meshSampler,i+2*step).r);
		vec3 v2 = vec3(texture1D(meshSampler,i+3*step).r, texture1D(meshSampler,i+4*step).r, texture1D(meshSampler,i+5*step).r);
		vec3 v3 = vec3(texture1D(meshSampler,i+6*step).r, texture1D(meshSampler,i+7*step).r, texture1D(meshSampler,i+8*step).r);
		if (rayIntersectsTriangle(ray_o, ray_t, v1, v2, v3) == 1) {
			return vec4(v3.z,0,0,1);
		}
		else {
			return vec4(1,1,0,1);
		}
//	}
}

void main() {
	// Output color = red 
	vec3 ray_o = vec3(pixel.x,pixel.y,0);//camera;
	vec3 ray_t = vec3(0,0,1);//normalize(viewplane_dis * vec3(0,0,1)+viewplane_scale * vec3(pixel.x,pixel.y,0));
	gl_FragColor = hit(ray_o, ray_t);
//	gl_FragColor = vec4(0,0,0,1);
//	gl_FragColor.r = texture1D(meshSampler,(pixel.x+1)/2).r;
}