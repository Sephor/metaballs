#version 330 core

uniform sampler2D shadowTexture;
uniform sampler2D thicknessTexture;
uniform sampler2D groundTexture;

in vec4 shadowPos;
in vec2 coords;

out vec4 color;

void main()
{
	//shadow calculations
	vec2 shadowTexCoord = shadowPos.xy / shadowPos.w;
	shadowTexCoord = (shadowTexCoord + vec2(1.0)) / 2.0;
	float thickness = texture(thicknessTexture, shadowTexCoord).x;
	float depth = texture(shadowTexture, shadowTexCoord).x;
	float depth2 = (shadowPos.z / shadowPos.w + 1.0) / 2.0;

	vec4 wColor = exp(-vec4(0.6, 0.2, 0.05, 3.0) * thickness * 3.5);
	color = texture(groundTexture, fract(coords * 5.0) * vec2(0.25, 1.0) + vec2(0.75, 0.0));
	if(depth < depth2 - 0.005)
	{
		color *= wColor;
	}
}
