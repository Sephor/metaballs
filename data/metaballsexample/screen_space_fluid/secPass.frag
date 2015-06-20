#version 150 
#extension GL_ARB_explicit_attrib_location : require

uniform sampler2D colorTexture;
uniform sampler2D depthTexture;
uniform samplerCube skybox;
uniform mat4 view;
uniform mat4 projectionInverted;
uniform mat4 viewInverted;
uniform float maxDepth;
uniform vec4 light_dir;
uniform bool blur;


in vec2 textCoord;
in vec3 v_sky;

out vec4 color;

vec4 naiveFilter(sampler2D text, vec2 uv, int size){
	vec2 texels = textureSize(text, 0);
	float offset = 1.0;
	vec4 sum = vec4(0.0);
	//float count = 0.0;
	vec4 centerValue = texture(text, uv) / ((size*2+1)*(size*2+1));
	for(int x = -size; x <= size; x+=1){
		for(int y = -size; y <= size; y+=1){
			vec4 nextValue = texture(text, uv +  vec2(x / texels.x * offset, y / texels.y * offset)) / ((size*2+1)*(size*2+1));
			if(abs(nextValue.x - centerValue.x) > 0.1)
			//if(nextValue.x == 1.0)
				sum += centerValue;
			else
				sum += nextValue;
			/*vec4 nextValue = texture(text, uv + vec2(x / texels.x * offset, y / texels.y * offset));
			if(nextValue.x == 1.0)
				count++;
			sum += nextValue; */
		}
	}
	//sum /= (size*2+1)*(size*2+1)-count;
	return sum ;
}

vec3 uvToEye(vec2 texCoord, float depth){
    float x = texCoord.x * 2.0 - 1.0;
    float y = texCoord.y * 2.0 - 1.0;
    vec4 clipPos = vec4(x , y, depth, 1.0);
    vec4 viewPos = projectionInverted * clipPos;
	return viewPos.xyz / viewPos.w;
}

vec3 getEyePos( sampler2D depthTexture, vec2 textCoord){
	float depth = naiveFilter(depthTexture, textCoord, 8).x;
	return uvToEye(textCoord, depth);
}

void main()
{
	vec2 texSize = textureSize(depthTexture, 0);
	
	vec2 texels = textureSize(depthTexture, 0);
	float texelSizeX = 1 / texels.x;
	float texelSizeY = 1 / texels.y;
	
	float depth = texture(depthTexture, textCoord).x;
	if(depth == 1.0)
		discard;
	
	if(blur)
		depth = naiveFilter(depthTexture, textCoord, 8).x;

	vec3 posEye = uvToEye(textCoord, depth);

	vec3 n = cross(dFdx(posEye), dFdy(posEye));
	
	n = normalize(n);
	
	color = vec4(n, 1.0);
	vec3 light= vec3(1.0, 0.0, 0.0);
	float diffuse = max(0.0, dot(n ,  light));
	color = vec4(0.1, 0.1, 0.5, 1.0) + vec4(vec3(0.4) * diffuse, 0.0);
}
