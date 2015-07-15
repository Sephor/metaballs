#version 330 core

uniform samplerCube skybox;

in vec3 v_sky;

out vec4 color;

void main()
{
	color = texture(skybox, normalize(v_sky) * vec3(1.0, -1.0, 1.0));
}
