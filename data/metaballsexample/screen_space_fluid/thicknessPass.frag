#version 330 core

in vec2 textcoord;
in float metaballRadius;

out vec4 color;

void main()
{
	vec2 normal = textcoord * 2.0 - 1.0;
	float r = dot(normal, normal);
	if (r > 1.0) discard;

	color = vec4(vec3(mix(0.1 * metaballRadius, 0.0, r)), 1.0);
}
