#version 150 core
#extension GL_ARB_explicit_attrib_location : require

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;

in vec2 textCoord;
out vec4 color;

void main()
{
	color = texture(colorTexture, textCoord);
	//color = texture(normalTexture, textCoord);
	//color = texture(depthTexture, textCoord);
}
