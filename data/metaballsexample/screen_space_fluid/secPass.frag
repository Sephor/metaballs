#version 150 
#extension GL_ARB_explicit_attrib_location : require

//uniform sampler2D colorTexture;
uniform sampler2D depthTexture;
uniform samplerCube skybox;
uniform mat4 view;
uniform mat4 projectionInverted;
uniform mat4 viewInverted;
uniform float maxDepth;
uniform vec4 light_dir;
uniform bool blur;
uniform mat4 projection;

uniform vec2 fov;
uniform vec2 focal;
uniform vec2 focal2;
uniform vec2 viewport;


in vec2 textCoord;
in vec3 v_sky;

//layout(location = 0) out vec4 color;

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

vec3 meanCurvature(vec2 pos) {
	// Width of one pixel
	vec2 dx = vec2(1.0 / viewport.x, 0.0);
	vec2 dy = vec2(0.0, 1.0 / viewport.y);

	// Central z value
	float zc =  texture(depthTexture, pos).x;

	// Take finite differences
	// Central differences give better results than one-sided here.
	// TODO better boundary conditions, possibly.
	// Remark: This is not easy, get to choose between bad oblique view smoothing
	// or merging of unrelated particles
	float zdxp = texture(depthTexture, pos + dx).x;
	float zdxn = texture(depthTexture, pos - dx).x;
	float zdx = 0.5 * (zdxp - zdxn);
	zdx = (zdxp == 0.0 || zdxn == 0.0) ? 0.0 : zdx;

	float zdyp = texture(depthTexture, pos + dy).x;
	float zdyn = texture(depthTexture, pos - dy).x;
	float zdy = 0.5 * (zdyp - zdyn);
	zdy = (zdyp == 0.0 || zdyn == 0.0) ? 0.0 : zdy;

	// Take second order finite differences
	float zdx2 = zdxp + zdxn - 2.0 * zc;
	float zdy2 = zdyp + zdyn - 2.0 * zc;

	// Second order finite differences, alternating variables
	float zdxpyp = texture(depthTexture, pos + dx + dy).x;
	float zdxnyn = texture(depthTexture, pos - dx - dy).x;
	float zdxpyn = texture(depthTexture, pos + dx - dy).x;
	float zdxnyp = texture(depthTexture, pos - dx + dy).x;
	float zdxy = (zdxpyp + zdxnyn - zdxpyn - zdxnyp) / 4.0;

	// Projection transform inversion terms
	float cx = 2.0 / (viewport.x * -projection[0][0]);
	float cy = 2.0 / (viewport.y * -projection[1][1]);

	// Normalization term
	float d = cy * cy * zdx * zdx + cx * cx * zdy * zdy + cx * cx * cy * cy * zc * zc;
	
	// Derivatives of said term
	float ddx = cy * cy * 2.0 * zdx * zdx2 + cx * cx * 2.0 * zdy * zdxy + cx * cx * cy * cy * 2.0 * zc * zdx;
	float ddy = cy * cy * 2.0 * zdx * zdxy + cx * cx * 2.0 * zdy * zdy2 + cx * cx * cy * cy * 2.0 * zc * zdy;

	// Temporary variables to calculate mean curvature
	float ex = 0.5 * zdx * ddx - zdx2 * d;
	float ey = 0.5 * zdy * ddy - zdy2 * d;

	// Finally, mean curvature
	float h = 0.5 * ((cy * ex + cx * ey) / pow(d, 1.5));
	
	return(vec3(zdx, zdy, h));
}

float bilateral(vec2 pos)
{
	const float epsilon = 0.01;
	const float posCoef = 1.0 / 16.0;
	float depth = texture(depthTexture, pos).x;
	vec2 texelSize = 1.0 / viewport;
	float sum = 0.0;
	float depthSum = 0.0;
	
	for(float x = -2; x <= 2; x++)
	{
		for(float y = -2; y <= 2; y++)
		{
			vec2 offset = vec2(x, y) * texelSize;
			vec2 nextPosition = pos + offset;
			float nextDepth = texture(depthTexture, nextPosition).x;
			
			float depthCoef = 1.0 / (epsilon * abs(depth - nextDepth));
			sum += depthCoef * posCoef;
			depthSum = depthCoef * nextDepth;
		}
	}
	
	return depthSum / sum;
}

void main()
{
	vec2 texSize = textureSize(depthTexture, 0);
	
	vec2 texels = textureSize(depthTexture, 0);
	float texelSizeX = 1 / texels.x;
	float texelSizeY = 1 / texels.y;
	
	float tempDepth = 0.0;
	tempDepth = texture(depthTexture, textCoord).x;
	if(tempDepth == 1.0)
		gl_FragDepth = 1.0;
	else
	{
		const float dt = 0.00055;
		const float dzt = 1000.0;
		vec3 dxyz = meanCurvature(textCoord);
		
		gl_FragDepth = tempDepth + dxyz.z * dt * (1.0 + (abs(dxyz.x) + abs(dxyz.y)) * dzt);
		gl_FragDepth = tempDepth + dxyz.z * dt * (1.0 + dxyz.x * dxyz.x + dxyz.y * dxyz.y);
	}
}
