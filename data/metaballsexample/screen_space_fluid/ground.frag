#version 330 core

uniform mat4 viewInverted;
uniform mat4 projectionInverted;
uniform mat4 viewShadow;
uniform mat4 projectionShadow;

uniform sampler2D shadowTexture;
uniform sampler2D thicknessTexture;
uniform sampler2D groundTexture;

in vec3 worldPos;
in vec4 shadowPos;
in vec2 coords;

out vec4 color;

void main()
{
	color = vec4(1.0);
	vec2 shadowTexCoord = shadowPos.xy / shadowPos.w;
	shadowTexCoord = (shadowTexCoord + vec2(1.0)) / 2.0;
	float thickness = texture(thicknessTexture, shadowTexCoord).x;
	float depth = texture(shadowTexture, shadowTexCoord).x;
	float depth2 = (shadowPos.z / shadowPos.w + 1.0) / 2.0;
	vec4 wColor = exp(-vec4(0.6f, 0.2f, 0.05f, 3.0f) * thickness * 2.0);
	if(depth < depth2 - 0.005)
	{
		color *= wColor;
	}
	color = texture(groundTexture, coords);
}
