#version 330 

//uniform sampler2D colorTexture;
uniform sampler2D depthTexture;
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

out vec4 color;

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

float lin(float depth)
{
	return (2 * near) / (far + near - depth * (far - near));
}

void main()
{
	gl_FragDepth = texture(depthTexture, textCoord).x;
	vec3 n = eyespaceNormal(textCoord);
	vec3 light = vec3(1.0, 0.0, 0.0);
	float diffuse = max(0.0, dot(n ,  light));
	color = vec4(0.1, 0.1, 0.5, 1.0) + vec4(vec3(0.4) * diffuse, 0.0);
	if(gl_FragDepth != 1.0)
	{
		vec3 light= vec3(1.0, 0.0, 0.0);
		float diffuse = max(0.0, dot(n ,  light));
		color = vec4(0.1, 0.1, 0.5, 1.0) + vec4(vec3(0.4) * diffuse, 0.0);
		vec3 r = reflect(v_sky, n);
		//r.y *= -1.0;
		color = texture(skybox, r);
		//color = vec4(vec3(gl_FragDepth), 1.0);
		//color = vec4(n, 1.0);
		//color = vec4(elemateNormal(textCoord), 1.0);
		//color = vec4(n, 1.0);
	}
	else
	{
		color = texture(skybox, vec3(v_sky.x, -v_sky.y, v_sky.z));
	}
	
	//color = vec4(vec3(gl_FragDepth), 1.0);
}