#version 330

//uniform sampler2D colorTexture;
uniform sampler2D depthTexture;
uniform sampler2D thicknessTexture;
uniform samplerCube skybox;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 viewInverted;
uniform mat4 projectionInverted;
uniform vec2 viewport;
uniform float near;
uniform float far;

in vec2 textCoord;
in vec3 v_sky;
in vec3 viewVector;

out vec4 color;

const vec3 waterColorf = vec3(64 / 255, 164/ 255, 223 / 255);

float lin(float depth)
{
	return (2 * near) / (far + near - depth * (far - near));
}

float fresnel(vec3 V, vec3 N)
{
	const float r0 = pow((1.0 - 1.333) / (1.0 + 1.333), 2.0);
	return r0 + (1.0 - r0) * pow((1.0 - max(dot(V, N), 0.0)), 5.0);
}

vec3 calcEye(vec2 pos)
{
	float z = texture(depthTexture, pos).x;
	pos = pos * 2.0 - vec2(1.0);
	return normalize((projectionInverted * vec4(pos, -z, 1.0)).xyz);
}

//UNUSED START
vec3 eyespaceNormal(vec2 pos) {
	// Width of one pixel
	vec2 dx = vec2(1.0 / viewport.x, 0.0);
	vec2 dy = vec2(0.0, 1.0 / viewport.y);

	// Central z
	float zc =  texture(depthTexture, pos).x;

	// Derivatives of z
	// For shading, one-sided only-the-one-that-works version
	float zdxp = texture(depthTexture, pos + dx).x;
	float zdxn = texture(depthTexture, pos - dx).x;
	float zdx = (zdxp == 0.0) ? (zdxn == 0.0 ? 0.0 : (zc - zdxn)) : (zdxp - zc);

	float zdyp = texture(depthTexture, pos + dy).x;
	float zdyn = texture(depthTexture, pos - dy).x;
	float zdy = (zdyp == 0.0) ? (zdyn == 0.0 ? 0.0 : (zc - zdyn)) : (zdyp - zc);

	// Projection inversion
	float cx = 2.0 / (viewport.x * -projection[0][0]);
	float cy = 2.0 / (viewport.y * -projection[1][1]);

	// Screenspace coordinates
	float sx = floor(pos.x * (viewport.x - 1.0));
	float sy = floor(pos.y * (viewport.y - 1.0));
	float wx = (viewport.x - 2.0 * sx) / (viewport.x * projection[0][0]);
	float wy = (viewport.y - 2.0 * sy) / (viewport.y * projection[1][1]);

	// Eyespace position derivatives
	vec3 pdx = normalize(vec3(cx * zc + wx * zdx, wy * zdx, zdx));
	vec3 pdy = normalize(vec3(wx * zdy, cy * zc + wy * zdy, zdy));

	return normalize(cross(pdx, pdy));
}
//UNUSED END

vec2 revert(vec2 pos)
{
    return pos * 2.0 - 1.0;
}

vec3 normal(vec2 pos)
{
    vec2 dx = vec2(1.0 / viewport.x, 0.0);
    vec2 dy = vec2(0.0, 1.0 / viewport.y);

    float z = texture(depthTexture, pos).x;
    float dxp = texture(depthTexture, pos + dx).x;
    float dxn = texture(depthTexture, pos - dx).x;
    float dyp = texture(depthTexture, pos + dy).x;
    float dyn = texture(depthTexture, pos - dy).x;

    vec4 vzFull = projectionInverted * vec4(revert(pos),z*2-1 ,1.0);
    vec3 vz = vzFull.xyz / vzFull.w;
    vec4 vxpFull = projectionInverted * vec4(revert(pos + dx), dxp*2-1, 1.0);
	vec3 vxp = vxpFull.xyz / vxpFull.w;
    vec4 vxnFull = projectionInverted * vec4(revert(pos - dx), dxn*2-1, 1.0);
	vec3 vxn = vxnFull.xyz / vxnFull.w;
    vec4 vypFull = projectionInverted * vec4(revert(pos + dy), dyp*2-1, 1.0);
	vec3 vyp = vypFull.xyz / vypFull.w;
    vec4 vynFull = projectionInverted * vec4(revert(pos - dy), dyn*2-1, 1.0);
	vec3 vyn = vynFull.xyz / vynFull.w;

	//TODO: Wenn differenz zu groß vxn/vyn benutzen
    return normalize(cross(vxp - vz, vyp - vz));
    //return normalize(cross(dFdy(vz), dFdx(vz)));
}

vec3 elemateNormal(vec2 pos)
{
    float depth = texture(depthTexture, pos).r;
	vec2 dx = vec2(1.0 / viewport.x, 0.0);
	vec2 dy = vec2(0.0, 1.0 / viewport.y);

    float height_left = texture(depthTexture, pos - dx).x;
    float height_right = texture(depthTexture, pos + dx).x;
    float height_up = texture(depthTexture, pos + dy).x;
    float height_down = texture(depthTexture, pos - dy).x;

    vec3 va,vb;

    va = mix(
            vec3(0.001*(0.01+height_right), 0.0, height_right - depth),
            vec3(0.001*(0.01+height_left), 0.0, depth - height_left),
            step(
                abs(height_left-depth),
                abs(height_right-depth)
            )
    );
    vb = mix(
        vec3(0.0, 0.001*(0.01+height_up), depth - height_up),
        vec3(0.0, 0.001*(0.01+height_down), height_down - depth),
        step(
            abs(height_down-depth),
            abs(height_up-depth)
        )
    );

    return normalize(cross(va, vb));
}

void main()
{
	gl_FragDepth = texture(depthTexture, textCoord).x;
	vec3 n = eyespaceNormal(textCoord);
	n = normal(textCoord);
	vec3 light = vec3(1.0, 0.0, 0.0);
	float diffuse = max(0.0, dot(n ,  light));
	color = vec4(0.1, 0.1, 0.5, 1.0) + vec4(vec3(0.4) * diffuse, 0.0);
	if(gl_FragDepth != 1.0)
	{
		vec3 worldSpaceNormal = normalize((viewInverted * vec4(n, 1.0)).xyz);
		float fresnelTerm = fresnel(normalize(viewVector), n);
		//fresnelTerm = fresnel(-calcEye(textCoord), n);
		float lambertTerm = max(0.0, dot(normalize(light - viewVector), n));
		
		/* reflect */
		vec3 r = reflect(normalize(v_sky), worldSpaceNormal);
		r *= -1.0;
		vec4 reflectColor = texture(skybox, r);
		
		/* refract */
		vec3 refr = refract(normalize(v_sky), worldSpaceNormal, 1.0/1.333);
		refr.y *= -1.0;
		vec4 refractColor = texture(skybox, refr);
		
		/* thickness */
		float thickness = texture(thicknessTexture, textCoord).x;
		
		//refractColor = mix(vec4(min(exp(vec3(1.0) - inverseWaterColor * vec3(thickness)), darkestWaterColor), 1.0), refractColor, exp(-thickness));
		//refractColor = vec4(exp(waterColor * vec3(thickness)) * 0.01, 1.0);
		refractColor = mix(vec4(waterColorf + vec3(0.2, 0.2, 0.4), 1.0), texture(skybox, refr), exp(-thickness));
		float strange = dot(worldSpaceNormal, 0.5 * (v_sky + normalize(light)));
		color = refractColor * (1.0 - fresnelTerm) +  reflectColor * fresnelTerm + min(0.4, lambertTerm);
		//color = vec4(viewVector, 1.0);
		//color = vec4(calcEye(textCoord), 1.0);
		//color = vec4(vec3(lambertTerm), 1.0);
	}
	else
	{
		color = texture(skybox, vec3(v_sky.x, -v_sky.y, v_sky.z));
	}
	//color = vec4(vec3(gl_FragDepth), 0.0);
}
