#version 330 core

uniform float sphere_radius; //TODO: braucht man nicht mehr
uniform mat4 projection;
uniform mat4 view; //TODO: braucht man nicht mehr
uniform vec4 light_dir; //TODO: braucht man nicht mehr
uniform float far;
uniform float near;

in vec2 textcoord;
in vec3 eyeSpacePos;
in float metaRadius;

vec3 normal;

void main()
{
	//calculate eyespace normal from textcoord
    vec2 tex = textcoord * 2.0 - vec2(1.0);
	float radius = dot(tex, tex);
	if (radius > 1.0) discard; // kill pixels outside circle

    radius = sqrt(1.0 - radius);

	//calculate depth
	vec4 fragmentPos = vec4(eyeSpacePos.xy, eyeSpacePos.z + radius * metaRadius, 1.0);
	vec4 clipSpacePos = projection * fragmentPos;
	gl_FragDepth = clipSpacePos.z / clipSpacePos.w;
}
