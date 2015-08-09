#version 330 core

//simmple verex shader, but extracts the radius from the w component

uniform mat4 view;

in vec4 in_vertex;

out float radius;

void main() {
    gl_Position = view * vec4(in_vertex.xyz, 1.0);
	radius = in_vertex.w;
}
