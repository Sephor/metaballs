#version 330

//uniform sampler2D colorTexture;
uniform sampler2D depthTexture;
uniform sampler2D thicknessTexture;
uniform samplerCube skybox;
uniform sampler2D shadowTexture;
uniform sampler2D groundTexture;
uniform sampler2D groundDepthTexture;
uniform sampler2D shadowThicknessTexture;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 viewInverted;
uniform mat4 projectionInverted;
uniform vec2 viewport;
uniform float near;
uniform float far;
uniform vec3 lightPos;
uniform vec3 eye;

uniform mat4 viewShadow;
uniform mat4 projectionShadow;

in vec2 textCoord;
in vec3 v_sky;
in vec3 viewVector;

out vec4 color;

const vec3 sunColor = vec3(0.99, 0.72, 0.07);

float lin(float depth)
{
	return (2 * near) / (far + near - depth * (far - near));
}

float fresnel(vec3 V, vec3 N)
{
	const float r0 = pow((1.0 - 1.333) / (1.0 + 1.333), 2.0);
	//return abs(dot(V, N));
	return r0 + (1.0 - r0) * pow((1.0 - max(dot(V, N), 0.0)), 5.0);
}

vec3 calcEye(vec2 pos)
{
	float z = texture(depthTexture, pos).x;
	pos = pos * 2.0 - vec2(1.0);
	return normalize((projectionInverted * vec4(pos, -z, 1.0)).xyz);
}

//convert the interval
vec2 revert(vec2 pos)
{
    return pos * 2.0 - 1.0;
}

//calculate the normal from the depth values
vec3 normal(vec2 pos)
{
	//one pixel in x/y direction
    vec2 dx = vec2(1.0 / viewport.x, 0.0);
    vec2 dy = vec2(0.0, 1.0 / viewport.y);

	//center depth
    float z = texture(depthTexture, pos).x;

	//change of depth in vertical/horizotal direction
    float zdxp = texture(depthTexture, pos + dx).x;
    float zdxn = texture(depthTexture, pos - dx).x;
    float zdyp = texture(depthTexture, pos + dy).x;
    float zdyn = texture(depthTexture, pos - dy).x;

	//vectors to the points
    vec4 vzFull = projectionInverted * vec4(revert(pos), z * 2 - 1 ,1.0);
    vec3 vz = vzFull.xyz / vzFull.w;

    vec4 vxpFull = projectionInverted * vec4(revert(pos + dx), zdxp * 2 - 1, 1.0);
	vec3 vxp = vxpFull.xyz / vxpFull.w;
    vec4 vypFull = projectionInverted * vec4(revert(pos + dy), zdyp * 2 - 1, 1.0);
	vec3 vyp = vypFull.xyz / vypFull.w;
    vec4 vxnFull = projectionInverted * vec4(revert(pos - dx), zdxn * 2 - 1, 1.0);
	vec3 vxn = vxnFull.xyz / vxnFull.w;
    vec4 vynFull = projectionInverted * vec4(revert(pos - dy), zdyn * 2 - 1, 1.0);
	vec3 vyn = vynFull.xyz / vynFull.w;

	//Take the vectors where the change in depth is the smallest. This ensures
	//that differences aren't taken across edges
	vec3 xTangent = (abs(z - zdxp) < abs(z - zdxn)) ? vxp - vz : vz - vxn;
	vec3 yTangent = (abs(z - zdyp) < abs(z - zdyn)) ? vyp - vz : vz - vyn;

	//calculate normal from tangent
    return normalize(cross(xTangent, yTangent));
}

void main()
{
	vec3 n = normal(textCoord);
	vec3 viewVector2 = calcEye(textCoord);
	float metaballDepth = texture(depthTexture, textCoord).x;
	float groundDepth = texture(groundDepthTexture, textCoord).x;

	//Draw metaballs if they are in front of the ground plane, the plane otherwise
	if(metaballDepth <= groundDepth)
	{
		gl_FragDepth = metaballDepth;
		vec3 worldSpaceNormal = normalize((vec4(n, 1.0) * view).xyz);
		float fresnelTerm = fresnel(-viewVector2, n);

		//tickness
		float thickness = texture(thicknessTexture, textCoord).x;

		//reflect
		vec3 r = reflect(normalize(v_sky), worldSpaceNormal);
		r.y *= -1.0;
		vec4 reflectColor = texture(skybox, r);

		//refract
		vec2 refr = (textCoord - n.xy * thickness);
		vec4 refractColor = texture(groundTexture, refr);

		//water color
		vec4 waterColor = exp(-vec4(0.6, 0.2, 0.05, 3.0) * thickness * 5.0);
		refractColor = mix(waterColor, refractColor, 0.3 + 0.7 * exp(-thickness));

		//specular
		vec4 specular = vec4(sunColor, 0.0) * pow(dot(worldSpaceNormal, 0.5 * -(v_sky + normalize(eye - lightPos))), 10.0);
		color = mix(refractColor, reflectColor, fresnelTerm) + max(vec4(0.0), specular);
	}
	else
	{
		gl_FragDepth = groundDepth;
		color = texture(groundTexture, textCoord);
	}

	if(metaballDepth == 1.0 && groundDepth == 1.0)
	{
		color = texture(skybox, vec3(v_sky.x, -v_sky.y, v_sky.z));
	}
}
