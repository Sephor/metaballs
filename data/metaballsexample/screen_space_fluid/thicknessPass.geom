#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 projection;
uniform float sphere_radius;

in float radius[];

out vec2 textcoord;
out float metaballRadius;

void main() {

	gl_Position = gl_in[0].gl_Position + vec4(-radius[0], radius[0], 0.0, 0.0);
    gl_Position = (projection * gl_Position);
	textcoord.x = 0.0;
	textcoord.y = 0.0;
	metaballRadius = radius[0];
	EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(radius[0], radius[0], 0.0, 0.0);
    gl_Position = (projection * gl_Position);
	textcoord.x = 1.0;
	textcoord.y = 0.0;
	metaballRadius = radius[0];
	EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(-radius[0], -radius[0], 0.0, 0.0);
    gl_Position = (projection * gl_Position);
	textcoord.x = 0.0;
	textcoord.y = 1.0;
	metaballRadius = radius[0];
	EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(radius[0], -radius[0], 0.0, 0.0);
	gl_Position = (projection * gl_Position);
	textcoord.x = 1.0;
	textcoord.y = 1.0;
	metaballRadius = radius[0];
	EmitVertex();

	EndPrimitive();
}
