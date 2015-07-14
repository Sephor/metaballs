#version 330 core

uniform mat4 view;
uniform mat4 projection;
uniform mat4 viewShadow;
uniform mat4 projectionShadow;

in vec3 in_vertex;

out vec3 worldPos;
out vec4 shadowPos;

void main() 
{
	worldPos = in_vertex;
	shadowPos =	projectionShadow * viewShadow * vec4(in_vertex, 1.0);
    gl_Position = projection * view * vec4(in_vertex, 1.0);
}
