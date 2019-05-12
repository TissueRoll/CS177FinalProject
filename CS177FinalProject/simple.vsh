#version 330 core

#define AMPLITUDE 1.0
#define PI 3.141592653589793238462643383279502

layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec4 v_color;

out vec3 f_pos;
out vec3 f_normal;
out vec4 f_color;

uniform mat4 m;
uniform mat4 mnormal;
uniform mat4 v;
uniform mat4 p;
uniform mat4 mvp;

// old stuff
// from https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
// from https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
// from https://stackoverflow.com/questions/39292925/glsl-calculating-normal-on-a-sphere-mesh-in-vertex-shader-using-noise-function-b


void main() {
	f_color = v_color;
	f_normal = mat3(mnormal) * v_normal;
	f_pos = (mvp * vec4(v_pos, 1.f)).xyz;
	gl_Position = mvp * vec4(v_pos, 1.f);
}
