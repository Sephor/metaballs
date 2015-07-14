#version 150 core

in vec2 in_vertex;
out vec2 textCoord;
void main() {
	gl_Position = vec4(in_vertex , 1.0, 1.0);
	textCoord = (in_vertex + 1) / 2;
}
