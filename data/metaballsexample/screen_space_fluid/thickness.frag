#version 330 core

//draw circles which very roughly estimate the thickness of a metaball.
//This doesn't need to be precice because the differences are very minor

in vec2 textcoord;
in float metaballRadius;

out vec4 color;

void main()
{
	vec2 normal = textcoord * 2.0 - 1.0;
	float r = dot(normal, normal);
	if (r > 1.0) discard;

	color = vec4(vec3(mix(1.0 * metaballRadius, 0.0, r)), 1.0);
}
