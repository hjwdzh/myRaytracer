#version 120
// Input vertex data, different for all executions of this shader.
attribute vec3 vertex;
varying vec2 pixel;
void main(){
	pixel = vertex.xy;
	gl_Position = vec4(vertex, 1.0);

}

