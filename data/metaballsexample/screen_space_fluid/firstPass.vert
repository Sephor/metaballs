#version 330 core

in ivec2 in_uv;

uniform sampler2D positionTexture;
uniform mat4 view;

out float radius;

void main() {
	vec4 posData = texelFetch(positionTexture, in_uv, 0);
    gl_Position = view * vec4(posData.xyz, 1.0);
	radius = posData.w;
}