#version 150 core
#extension GL_ARB_explicit_attrib_location : require

uniform sampler2D colorTexture;
uniform sampler2D depthTexture;
uniform mat4 projectionInverted;
uniform float maxDepth;
uniform float texelSize;

in vec2 textCoord;
out vec4 color;


vec3 uvToEye(vec2 texCoord, float depth){
    float x = texCoord.x * 2.0 - 1.0;
    float y = texCoord.y * 2.0 - 1.0;
    vec4 clipPos = vec4(x , y, depth, 1.0f);
    vec4 viewPos = projectionInverted * clipPos;
    return viewPos.xyz / viewPos.w;
}

vec3 getEyePos( sampler2D depthTexture, vec2 textCoord){
	float depth = texture(depthTexture, textCoord).x;
	return uvToEye(textCoord, depth);
}

void main()
{
	float depth = texture(depthTexture, textCoord).x;
	if(depth >= maxDepth) {
		discard;
		return;
	}
	
	vec3 posEye = uvToEye(textCoord, depth);
	vec3 ddx = getEyePos( depthTexture, textCoord + vec2(texelSize, 0) ) -posEye;
	vec3 ddx2 = posEye -getEyePos( depthTexture, textCoord + vec2(-texelSize, 0) );
	if (abs(ddx.z) > abs(ddx2.z)) {
		ddx = ddx2;
	}
	vec3 ddy = getEyePos(depthTexture, textCoord[0] + vec2(0, texelSize)) -posEye;
	vec3 ddy2 = posEye -getEyePos(depthTexture, textCoord + vec2(0, -texelSize));
	if (abs(ddy2.z) < abs(ddy.z)) {
		ddy = ddy2;
	}
	// calculate normal
	vec3 n = cross(ddx, ddy);
	n = normalize(n);
	
	color = vec4(n, 1.0); 
}
