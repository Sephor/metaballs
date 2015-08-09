#version 330 core

//Creates screen alligned planes out of input vertecies

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 projection;

in float radius[];

out vec2 textcoord;
out vec3 viewSpacePos;
out float metaRadius;

void main() {
	//top left vertex
	gl_Position = gl_in[0].gl_Position + vec4(-radius[0], radius[0], 0.0, 0.0);
	viewSpacePos = gl_Position.xyz;
    gl_Position = (projection * gl_Position);
	textcoord.x = 0.0;
	textcoord.y = 0.0;
    metaRadius = radius[0];
	EmitVertex();

	//top right vertex
    gl_Position = gl_in[0].gl_Position + vec4(radius[0], radius[0], 0.0, 0.0);
	viewSpacePos = gl_Position.xyz;
    gl_Position = (projection * gl_Position);
	textcoord.x = 1.0;
	textcoord.y = 0.0;
    metaRadius = radius[0];
	EmitVertex();

	//bottom left vertex
    gl_Position = gl_in[0].gl_Position + vec4(-radius[0], -radius[0], 0.0, 0.0);
	viewSpacePos = gl_Position.xyz;
    gl_Position = (projection * gl_Position);
	textcoord.x = 0.0;
	textcoord.y = 1.0;
    metaRadius = radius[0];
	EmitVertex();

	//bottom right vertex
    gl_Position = gl_in[0].gl_Position + vec4(radius[0], -radius[0], 0.0, 0.0);
    viewSpacePos = gl_Position.xyz;
	gl_Position = (projection * gl_Position);
	textcoord.x = 1.0;
	textcoord.y = 1.0;
    metaRadius = radius[0];
	EmitVertex();

	EndPrimitive();
}
