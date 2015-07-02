#version 150 core

uniform mat4 view;

in vec4 in_vertex;

out float radius;

void main() {
    gl_Position = view * vec4(in_vertex.xyz, 1.0);
	radius = in_vertex.w;
}
