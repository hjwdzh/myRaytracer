#version 120

uniform vec3 camera;
uniform float viewplane_dis;
uniform float viewplane_scale;
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
	if (rayIntersectsTriangle(ray_o, ray_t, vec3(-0.5,-0.5,4),vec3(0,0.5,4),vec3(0.5,-0.5,4)) == 1) {
		return vec4(1,0,0,1);
	}
	else {
		return vec4(1,1,0,1);
	}
}

void main() {
	// Output color = red 
	vec3 ray_o = camera;
	vec3 ray_t = normalize(viewplane_dis * vec3(0,0,1)+viewplane_scale * vec3(pixel.x,pixel.y,0));
	gl_FragColor = hit(ray_o, ray_t);
}