#version 150 
#extension GL_ARB_explicit_attrib_location : require

uniform sampler2D colorTexture;
uniform sampler2D depthTexture;
uniform mat4 view;
uniform mat4 projectionInverted;
uniform mat4 viewInverted;
uniform float maxDepth;
uniform vec4 light_dir;


in vec2 textCoord;

out vec4 color;

vec4 naiveFilter(sampler2D text, vec2 uv, int size){
	vec2 texels = textureSize(text, 0);
	float offset = 8.0;
	vec4 sum = vec4(0.0);
	vec4 centerValue = texture(text, uv);
	for(int x = -size; x <= size; x+=1){
		for(int y = -size; y <= size; y+=1){
			vec4 nextValue = texture(text, uv +  vec2(x / texels.x * offset, y / texels.y * offset)) / ((size*2+1)*(size*2+1));
			if(0.1 > abs(nextValue.x - centerValue.x))
				sum += centerValue;
			if(0.1 <= abs(nextValue.x - centerValue.x))
				sum += nextValue;
		}
	}
	return sum ;
}

vec3 uvToEye(vec2 texCoord, float depth){
    float x = texCoord.x * 2.0 - 1.0;
    float y = texCoord.y * 2.0 - 1.0;
    vec4 clipPos = vec4(x , y, depth, 1.0f);
    vec4 viewPos = projectionInverted * clipPos;
	return viewPos.xyz / viewPos.w;
}

vec3 getEyePos( sampler2D depthTexture, vec2 textCoord){
	float depth = naiveFilter(depthTexture, textCoord, 4).x;
	//float depth = texture(depthTexture, textCoord).x;
	return uvToEye(textCoord, depth);
}

void main()
{
	float depth = texture(depthTexture, textCoord).x;
	if(depth >= maxDepth) {
			discard;
	}
	
	vec2 texels = textureSize(depthTexture, 0);
	float texelSizeX = 1 / texels.x;
	float texelSizeY = 1 / texels.y;
	
	depth = naiveFilter(depthTexture, textCoord, 4).x;

	vec3 posEye = uvToEye(textCoord, depth);
	vec3 ddx = getEyePos( depthTexture, textCoord + vec2(texelSizeX, 0) ) -posEye;
	vec3 ddx2 = posEye - getEyePos( depthTexture, textCoord + vec2(-texelSizeX, 0) );
	if (abs(ddx.z) > abs(ddx2.z)) {
		ddx = ddx2;
	}
	vec3 ddy = getEyePos( depthTexture, textCoord + vec2(0, texelSizeY)) -posEye;
	vec3 ddy2 = posEye - getEyePos( depthTexture, textCoord + vec2(0, -texelSizeY));
	if (abs(ddy2.z) < abs(ddy.z)) {
		ddy = ddy2;
	}
	// calculate normal
	vec3 n = cross(ddx, ddy);
	
	n = normalize(n);
	
	color = vec4(n, 1.0); 
	//color = vec4(vec3(depth), 1.0);
	/*vec3 light= vec3(1.0, 0.0, 0.0);
	float diffuse = max(0.0, dot(n ,  light));
	color = vec4(0.1, 0.1, 0.5, 1.0);
	color += vec4(0.1, 0.1, 0.5, 1.0)* diffuse;
	color += vec4(0.1, 0.1, 0.1, 1.0); */
	
	
}
