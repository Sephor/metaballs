#version 150 core
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) out vec4 color;
layout(location = 1) out vec3 normal; 

void main()
{
	color = vec4(0.5, 0.5, 0.5, 1.0);
}
