#version 150 core

in vec2 in_vertex;

out vec2 textCoord;
out vec3 v_sky;

uniform mat4 projectionInverted;
uniform mat4 view;

void main() {
	v_sky = (projectionInverted * vec4(in_vertex, 0.0, 1.0) * view).xyz;
	gl_Position = vec4(in_vertex, 0.0, 1.0);
	textCoord = (in_vertex + vec2(1.0)) / 2;
}