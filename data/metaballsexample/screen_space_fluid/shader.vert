#version 150 core

in vec4 pos;

uniform mat4 view;

void main() {

    gl_Position = view * pos;
}
