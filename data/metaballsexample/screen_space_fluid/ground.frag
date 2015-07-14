#version 330 core

uniform mat4 viewInverted;
uniform mat4 projectionInverted;
uniform mat4 viewShadow;
uniform mat4 projectionShadow;

uniform sampler2D shadowTexture;

in vec3 worldPos;
in vec4 shadowPos;

out vec4 color;

void main()
{
	color = vec4(1.0);
	vec2 shadowTexCoord = shadowPos.xy / shadowPos.w;
	shadowTexCoord = (shadowTexCoord + vec2(1.0)) / 2.0;
	float depth = texture(shadowTexture, shadowTexCoord).x;
	float depth2 = (shadowPos.z / shadowPos.w + 1.0) / 2.0;
	if(depth < depth2 - 0.005)
	{
		color *= vec4(0.5, 0.5, 0.5, 1.0);
	}
}
