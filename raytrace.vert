#version 120
// Input vertex data, different for all executions of this shader.
attribute vec3 vertex;
uniform vec3 camera;
uniform float viewplane_dis;
uniform float viewplane_scale;
varying vec2 pixel;

uniform vec3 triangles[3];
uniform float step;

void main(){
	pixel.xy = vertex.xy;
	gl_Position = vec4(vertex, 1.0);
}

