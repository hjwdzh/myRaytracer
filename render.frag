#version 120

varying vec2 pixel;

uniform sampler2D renderSampler;

void main() {
	gl_FragColor.rgb = texture2D(renderSampler, pixel).rgb;
	gl_FragColor.a = 1;
}