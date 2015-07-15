#version 330 core

uniform mat4 view;
uniform mat4 projectionInverted;

in vec2 in_vertex;

out vec3 v_sky;

void main() 
{
    v_sky = (projectionInverted * vec4(in_vertex, 1.0, 1.0) * view).xyz;
	gl_Position = vec4(in_vertex, 1.0, 1.0);
}
