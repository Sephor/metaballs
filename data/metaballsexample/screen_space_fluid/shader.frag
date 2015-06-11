#version 150 core

uniform vec3 eye_pos;
uniform float sphere_radius;
uniform mat4 projection;
uniform vec4 light_dir;

in vec2 textcoord;
out vec4 color;

vec3 normal; 

void main()
{

	vec4 light = normalize(light_dir);
	//calculate eyespace normal from textcoord	
	normal.xy = textcoord*2.0-1.0;
	float radius = dot(normal.xy, normal.xy);
	if (radius > 1.0) discard; // kill pixels outside circle
	
	normal.z = -sqrt(1.0 - radius);
	
	//calculate depth
	vec4 fragmentPos = vec4(eye_pos + normal*sphere_radius, 1.0);
	vec4 clipSpacePos = fragmentPos * projection;
	float fragDepth = clipSpacePos.z / clipSpacePos.w;
	
	float diffuse = max(0.0, dot(vec4(normal, 1.0), light));
	color = vec4(0.5, 0.5, 0.5, 1.0);
	color *= diffuse;


}
