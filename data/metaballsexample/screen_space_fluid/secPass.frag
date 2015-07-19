#version 330

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
uniform float timeStep;

uniform vec2 fov;
uniform vec2 focal;
uniform vec2 focal2;
uniform vec2 viewport;

in vec2 textCoord;
in vec3 v_sky;

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
	zdx = (zdxp == 1.0) ? zc - zdxn : zdx;
	zdx = (zdxn == 1.0) ? zdxp - zc : zdx;
	//zdx = (zdxp == 1.0 || zdxn == 1.0) ? 0.0 : zdx;

	float zdyp = texture(depthTexture, pos + dy).x;
	float zdyn = texture(depthTexture, pos - dy).x;
	float zdy = 0.5 * (zdyp - zdyn);
	zdy = (zdyp == 1.0) ? zc - zdyn : zdy;
	zdy = (zdyn == 1.0) ? zdyp - zc : zdy;
	//zdy = (zdyp == 1.0 || zdyn == 1.0) ? 0.0 : zdy;

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

void main()
{
	float tempDepth = texture(depthTexture, textCoord).x;
	if(tempDepth == 1.0)
		gl_FragDepth = 1.0;
	else
	{
		const float dt = 0.0002;
		
		vec3 dxyz = meanCurvature(textCoord);
		gl_FragDepth = tempDepth + dxyz.z * timeStep * (1.0 + dxyz.x * dxyz.x + dxyz.y * dxyz.y);
	}
}
