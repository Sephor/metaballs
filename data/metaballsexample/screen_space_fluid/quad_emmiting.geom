#version 150 core

layout(points) in;
layout(triangle_strip, max_vertices = 6) out;

uniform mat4 projection;
uniform float sphere_radius;

out vec2 textcoord;


void main() {	
	
    gl_Position = gl_in[0].gl_Position + vec4(-sphere_radius, -sphere_radius, 0.0, 0.0);
    gl_Position = (projection * gl_Position);
	textcoord.x = 0.0;
	textcoord.y = 1.0;
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(-sphere_radius, sphere_radius, 0.0, 0.0);
    gl_Position = (projection * gl_Position);
	textcoord.x = 0.0;
	textcoord.y = 0.0;
	EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(sphere_radius, sphere_radius, 0.0, 0.0);
    gl_Position = (projection * gl_Position);
	textcoord.x = 1.0;
	textcoord.y = 0.0;
	EmitVertex();
	
	EndPrimitive();
	
	gl_Position = gl_in[0].gl_Position + vec4(-sphere_radius, -sphere_radius, 0.0, 0.0);
    gl_Position = projection * gl_Position;
	textcoord.x = 0.0;
	textcoord.y = 1.0;
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(sphere_radius, sphere_radius, 0.0, 0.0);
    gl_Position = (projection * gl_Position);
	textcoord.x = 1.0;
	textcoord.y = 0.0;
	EmitVertex();	

    gl_Position = gl_in[0].gl_Position + vec4(sphere_radius, -sphere_radius, 0.0, 0.0);
    gl_Position = (projection * gl_Position);
	textcoord.x = 1.0;
	textcoord.y = 1.0;
	EmitVertex();

    EndPrimitive();
}