#version 150 
#extension GL_ARB_explicit_attrib_location : require

uniform sampler2D colorTexture;
uniform sampler2D depthTexture;

uniform mat4 projectionInverted;
uniform mat4 viewInverted;
uniform float maxDepth;

in vec2 textCoord;

out vec4 color;
out vec4 sDepth;

void main()
{
	color = texture(depthTexture, textCoord);
}
