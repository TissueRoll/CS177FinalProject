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

// from https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
vec3 permute(vec3 x) { return mod(((x*34.0) + 1.0)*x, 289.0); }

float snoise(vec2 v) {
	const vec4 C = vec4(0.211324865405187, 0.366025403784439,
		-0.577350269189626, 0.024390243902439);
	vec2 i = floor(v + dot(v, C.yy));
	vec2 x0 = v - i + dot(i, C.xx);
	vec2 i1;
	i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
	vec4 x12 = x0.xyxy + C.xxzz;
	x12.xy -= i1;
	i = mod(i, 289.0);
	vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0))
		+ i.x + vec3(0.0, i1.x, 1.0));
	vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x12.xy, x12.xy),
		dot(x12.zw, x12.zw)), 0.0);
	m = m * m;
	m = m * m;
	vec3 x = 2.0 * fract(p * C.www) - 1.0;
	vec3 h = abs(x) - 0.5;
	vec3 ox = floor(x + 0.5);
	vec3 a0 = x - ox;
	m *= 1.79284291400159 - 0.85373472095314 * (a0*a0 + h * h);
	vec3 g;
	g.x = a0.x  * x0.x + h.x  * x0.y;
	g.yz = a0.yz * x12.xz + h.yz * x12.yw;
	return 130.0 * dot(m, g);
}

vec3 getPos(vec3 pos) {
	return pos * snoise(cross(pos*69, pos*420).xy);
}

// from https://stackoverflow.com/questions/39292925/glsl-calculating-normal-on-a-sphere-mesh-in-vertex-shader-using-noise-function-b
vec3 calcNormal(vec3 pos)
{
	float theta = .00001;
	vec3 vecTangent = normalize(cross(pos, vec3(1.0, 0.0, 0.0))
		+ cross(pos, vec3(0.0, 1.0, 0.0)));
	vec3 vecBitangent = normalize(cross(vecTangent, pos));
	vec3 ptTangentSample = getPos(normalize(pos + theta * normalize(vecTangent)));
	vec3 ptBitangentSample = getPos(normalize(pos + theta * normalize(vecBitangent)));

	return normalize(cross(ptTangentSample - pos, ptBitangentSample - pos));
}

void main() {
	float len = length(v_pos);
	vec4 vc = vec4(v_pos, 1.f);
	vec3 temp = v_pos + calcNormal(v_pos*time*0.0001) * 0.07;
	gl_Position = mvp * vec4(temp,1.f);
	gl_Position = mvp * vec4(v_pos, 1.f);
}
