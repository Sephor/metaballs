#version 150 core

uniform vec3 eye_pos;
uniform float sphere_radius;
uniform mat4 projection;
uniform mat4 view;
uniform vec4 light_dir;

in vec2 textcoord;
in vec3 eyeSpacePos;
out vec4 color;
vec3 normal; 

void main()
{
	vec4 light = normalize(light_dir);
	//calculate eyespace normal from textcoord	
	normal.xy = textcoord*2.0 - 1.0;
	float radius = dot(normal.xy, normal.xy);
	if (radius > 1.0) discard; // kill pixels outside circle
	
	normal.z = -sqrt(1.0 - radius);
	
	//calculate depth
	vec4 fragmentPos = vec4( eyeSpacePos + normal*sphere_radius, 1.0);
	vec4 clipSpacePos = projection * fragmentPos;
	gl_FragDepth = clipSpacePos.z / clipSpacePos.w;
	
	float diffuse = max(0.0, dot(vec4(normal, 1.0), light));
	color = vec4(0.5, 0.5, 0.5, 1.0);
	color *= diffuse;
	color = vec4(gl_FragCoord.z / gl_FragCoord.w);
}
