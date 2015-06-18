#version 150 core

uniform mat4 view;

in vec4 in_vertex;

void main() {
    gl_Position = view * in_vertex;
}
