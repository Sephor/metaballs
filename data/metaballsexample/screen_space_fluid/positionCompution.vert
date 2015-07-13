#version 330 core

in vec2 in_uv;

uniform sampler2D positionTexture;
uniform sampler2D velocityTexture;
uniform sampler2D particleInfoTexture;

void main(){
	//gl_Position = vec4((in_uv+1 )/2, 0.0, 1.0); 
}