#version 330 core
#extension GL_ARB_explicit_attrib_location : require

uniform mat4 view;
uniform mat4 projection;
uniform mat4 viewShadow;
uniform mat4 projectionShadow;

layout(location = 0) in vec3 in_vertex;
layout(location = 1) in vec2 texCoords;

out vec3 worldPos;
out vec4 shadowPos;
out vec2 coords;

void main()
{
	worldPos = in_vertex;
	shadowPos =	projectionShadow * viewShadow * vec4(in_vertex, 1.0);
	coords = texCoords;
    gl_Position = projection * view * vec4(in_vertex, 1.0);
}
