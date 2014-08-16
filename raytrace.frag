#version 120
varying vec2 pixel;

void main()
{
	// Output color = red 
	gl_FragColor = vec4((pixel.x+1)/2,(pixel.y+1)/2,0,1);
}