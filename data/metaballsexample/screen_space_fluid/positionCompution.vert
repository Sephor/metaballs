#version 330 core

uniform sampler2D positionTexture;
uniform sampler2D velocityTexture;
uniform sampler2D particleInfoTexture;

in vec2 a_vertex;

void main(){
	gl_Position = vec4(a_vertex, 0.0, 1.0);
}
