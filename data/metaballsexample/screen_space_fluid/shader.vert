#version 150 core

in vec4 pos;

uniform mat4 viewProjection;

void main() {

    gl_Position = viewProjection * pos;
}
