#version 330 core

uniform mat4 projection;

in vec2 textcoord;
in vec3 viewSpacePos;
in float metaRadius;

vec3 normal;

void main()
{
	//calculate eyespace normal from textcoord
    vec2 position = textcoord * 2.0 - vec2(1.0);
	float sqRadius = dot(position, position);
	if (sqRadius > 1.0) discard; // kill fragments outside circle

    float radiusOffset = sqrt(1.0 - sqRadius);

	//calculate depth
	vec4 fragmentPos = vec4(viewSpacePos.xy, viewSpacePos.z + radiusOffset * metaRadius, 1.0);
	vec4 clipSpacePos = projection * fragmentPos;
	gl_FragDepth = clipSpacePos.z / clipSpacePos.w;
}