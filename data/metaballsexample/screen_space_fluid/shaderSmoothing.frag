#version 150 core
#extension GL_ARB_explicit_attrib_location : require

uniform sampler2D colorTexture;
uniform sampler2D depthTexture;
uniform mat4 projectionInverted;
uniform mat4 viewInverted;
uniform float maxDepth;
uniform float texelSize;

in vec2 textCoord;
out vec4 color;


vec3 uvToEye(vec2 texCoord, float depth){
    float x = texCoord.x * 2.0 - 1.0;
    float y = texCoord.y * 2.0 - 1.0;
    vec4 clipPos = vec4(x , y, depth, 1.0f);
    vec4 viewPos = projectionInverted * clipPos;
	return -viewPos.xyz / viewPos.w;
}

vec3 getEyePos( sampler2D depthTexture, vec2 textCoord){
	float depth = texture(depthTexture, textCoord).x;
	return uvToEye(textCoord, depth);
}


float bilateralFilteredDepth(sampler2D depthTexture, vec2 textCoord, float filterRadius, float blurScale, float blurDepthFalloff){
	float depth = texture(depthTexture, textCoord).x;
	float sum = 0;
	float wsum = 0;
	for(float x=-filterRadius; x<=filterRadius; x+=1.0) {
		for(float x=-filterRadius; x<=filterRadius; x+=1.0) {
		float sample = texture(depthTexture, textCoord + x*vec2(1.0 , 0.0)).x;
		// spatial domain
		float r = x * blurScale;
		float w = exp(-r*r);
		// range domain
		float r2 = (sample -depth) * blurDepthFalloff;
		float g = exp(-r2*r2);
		sum += sample * w * g;
		wsum += w * g;
		}
	}
	for(float y=-filterRadius; y<=filterRadius; y+=1.0) {
		float sample = texture(depthTexture, textCoord + y*vec2(0.0 , 1.0)).x;
		// spatial domain
		float r = y * blurScale;
		float w = exp(-r*r);
		// range domain
		float r2 = (sample -depth) * blurDepthFalloff;
		float g = exp(-r2*r2);
		sum += sample * w * g;
		wsum += w * g;
	}

	if (wsum > 0.0)
		sum /= wsum;
	return sum;
}

float gaussianBlur(sampler2D depthTexture, vec2 textCoord){
	float depth = 0.0;
	vec2 v_blurTexCoords[14];
	
	v_blurTexCoords[ 0] = textCoord + vec2(-0.028, 0.0);
    v_blurTexCoords[ 1] = textCoord + vec2(-0.024, 0.0);
    v_blurTexCoords[ 2] = textCoord + vec2(-0.020, 0.0);
    v_blurTexCoords[ 3] = textCoord + vec2(-0.016, 0.0);
    v_blurTexCoords[ 4] = textCoord + vec2(-0.012, 0.0);
    v_blurTexCoords[ 5] = textCoord + vec2(-0.008, 0.0);
    v_blurTexCoords[ 6] = textCoord + vec2(-0.004, 0.0);
    v_blurTexCoords[ 7] = textCoord + vec2( 0.004, 0.0);
    v_blurTexCoords[ 8] = textCoord + vec2( 0.008, 0.0);
    v_blurTexCoords[ 9] = textCoord + vec2( 0.012, 0.0);
    v_blurTexCoords[10] = textCoord + vec2( 0.016, 0.0);
    v_blurTexCoords[11] = textCoord + vec2( 0.020, 0.0);
    v_blurTexCoords[12] = textCoord + vec2( 0.024, 0.0);
    v_blurTexCoords[13] = textCoord + vec2( 0.028, 0.0);
	
    depth += texture2D(depthTexture, v_blurTexCoords[ 0]).x*0.0044299121055113265;
    depth += texture2D(depthTexture, v_blurTexCoords[ 1]).x*0.00895781211794;
    depth += texture2D(depthTexture, v_blurTexCoords[ 2]).x*0.0215963866053;
    depth += texture2D(depthTexture, v_blurTexCoords[ 3]).x*0.0443683338718;
    depth += texture2D(depthTexture, v_blurTexCoords[ 4]).x*0.0776744219933;
    depth += texture2D(depthTexture, v_blurTexCoords[ 5]).x*0.115876621105;
    depth += texture2D(depthTexture, v_blurTexCoords[ 6]).x*0.147308056121;
    depth += texture2D(depthTexture, textCoord         ).x*0.159576912161;
    depth += texture2D(depthTexture, v_blurTexCoords[ 7]).x*0.147308056121;
    depth += texture2D(depthTexture, v_blurTexCoords[ 8]).x*0.115876621105;
    depth += texture2D(depthTexture, v_blurTexCoords[ 9]).x*0.0776744219933;
    depth += texture2D(depthTexture, v_blurTexCoords[10]).x*0.0443683338718;
    depth += texture2D(depthTexture, v_blurTexCoords[11]).x*0.0215963866053;
    depth += texture2D(depthTexture, v_blurTexCoords[12]).x*0.00895781211794;
    depth += texture2D(depthTexture, v_blurTexCoords[13]).x*0.0044299121055113265;
	return depth;
}
void main()
{
	float depth = gaussianBlur(depthTexture, textCoord);
	//float depth = bilateralFilteredDepth(depthTexture, textCoord, 3.0, 5.0, 1.0);	
	//float depth = texture(depthTexture, textCoord).x;
	if(depth >= maxDepth) {
		discard;
		return;
	}
    
	vec3 posEye = uvToEye(textCoord, depth);
	vec3 ddx = getEyePos( depthTexture, textCoord + vec2(texelSize, 0) ) -posEye;
	vec3 ddx2 = posEye - getEyePos( depthTexture, textCoord + vec2(-texelSize, 0) );
	if (abs(ddx.z) > abs(ddx2.z)) {
		ddx = ddx2;
	}
	vec3 ddy = getEyePos( depthTexture, textCoord + vec2(0, texelSize)) -posEye;
	vec3 ddy2 = posEye - getEyePos( depthTexture, textCoord + vec2(0, -texelSize));
	if (abs(ddy2.z) < abs(ddy.z)) {
		ddy = ddy2;
	}
	// calculate normal
	vec3 n = cross(ddx, ddy);
	
	n = normalize(n);
	
	color = vec4(n, 1.0); 
	//color = vec4(depth);
}
