#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 projection;

in float radius[];

out vec2 textcoord;
out vec3 eyeSpacePos;
out float metaRadius;

void main() {
	gl_Position = gl_in[0].gl_Position + vec4(-radius[0], radius[0], 0.0, 0.0);
	eyeSpacePos = gl_Position.xyz;
    gl_Position = (projection * gl_Position);
	textcoord.x = 0.0;
	textcoord.y = 0.0;
    metaRadius = radius[0];
	EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(radius[0], radius[0], 0.0, 0.0);
	eyeSpacePos = gl_Position.xyz;
    gl_Position = (projection * gl_Position);
	textcoord.x = 1.0;
	textcoord.y = 0.0;
    metaRadius = radius[0];
	EmitVertex();
	
    gl_Position = gl_in[0].gl_Position + vec4(-radius[0], -radius[0], 0.0, 0.0);
	eyeSpacePos = gl_Position.xyz;
    gl_Position = (projection * gl_Position);
	textcoord.x = 0.0;
	textcoord.y = 1.0;
    metaRadius = radius[0];
	EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(radius[0], -radius[0], 0.0, 0.0);
    eyeSpacePos = gl_Position.xyz;
	gl_Position = (projection * gl_Position);
	textcoord.x = 1.0;
	textcoord.y = 1.0;
    metaRadius = radius[0];
	EmitVertex();

	EndPrimitive();
}