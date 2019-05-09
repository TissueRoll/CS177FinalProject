#version 330 core

#define AMPLITUDE 1.0
#define PI 3.141592653589793238462643383279502

layout(location = 0) in vec3 v_pos;

uniform float time;
uniform mat4 m;
uniform mat4 v;
uniform mat4 p;
uniform mat4 mvp;

// from https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
float rand(vec2 co) {
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
	float len = length(v_pos);
	vec4 vc = vec4(v_pos, 1.f);
	vec3 temp = v_pos + normalize(v_pos)*sin(2.0 * PI * len + time) * rand(vec2(420,69)) * 0.03;
	gl_Position = mvp * vec4(temp,1.f);
	
}
