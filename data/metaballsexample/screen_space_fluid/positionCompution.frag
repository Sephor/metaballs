#version 330 core

uniform sampler2D positionTexture;
uniform sampler2D velocityTexture;
uniform sampler2D particleInfoTexture;

out vec4 position;

void main(){
	ivec2 uv = ivec2(gl_FragCoord.xy);
	position = texelFetch(positionTexture, uv , 0);
	position += texelFetch(velocityTexture, uv , 0);
}