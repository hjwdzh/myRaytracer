#version 120
// Input vertex data, different for all executions of this shader.
attribute vec3 vertex;
varying vec2 pixel;

void main(){
	pixel.xy = (vertex.xy + vec2(1,1))*0.5;
	gl_Position = vec4(vertex, 1.0);
}

