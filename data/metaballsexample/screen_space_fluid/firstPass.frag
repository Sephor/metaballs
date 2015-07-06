#version 330 core

uniform float sphere_radius;
uniform mat4 projection;
uniform mat4 view;
uniform vec4 light_dir;
uniform float far;
uniform float near;

in vec2 textcoord;
in vec3 eyeSpacePos;

//layout(location = 0) out vec4 color;

vec3 normal; 

void main()
{
	vec3 light = normalize(light_dir).xyz;
	
	//calculate eyespace normal from textcoord	
	normal.xy = textcoord*2.0 - 1.0;
	float radius = dot(normal.xy, normal.xy);
	if (radius > 1.0) discard; // kill pixels outside circle
	
	normal.z = -sqrt(1.0 - radius);
	
	//calculate depth
	vec4 fragmentPos = vec4( eyeSpacePos - normal.xyz*sphere_radius, 1.0);
	vec4 clipSpacePos = projection * fragmentPos;
	gl_FragDepth = clipSpacePos.z / clipSpacePos.w;
}
