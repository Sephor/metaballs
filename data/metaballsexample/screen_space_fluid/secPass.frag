#version 330

//smooth the depthTexture with mean curvature flow

uniform sampler2D depthTexture;
uniform mat4 view;
uniform mat4 projection;
uniform float timeStep;

uniform vec2 viewport;

in vec2 textCoord;

vec3 meanCurvature(vec2 pos) {
	//This is an implements of mean curvature flow as found in the paper
	//"Screen Space Fluid Rendering with Curvature Flow" by Wladimir J. van der Laan,
	//Simon Green, and Miguel Sainz (2009)

	//Width of one pixel
	vec2 dx = vec2(1.0 / viewport.x, 0.0);
	vec2 dy = vec2(0.0, 1.0 / viewport.y);

	//Center depth
	float zc =  texture(depthTexture, pos).x;

	//Finite diferences for x
	//Let z(x) be the depth at horizontal position x and dx a small enough change
	//in x (e.g. one pixel), then zdx(x) can be approximated as:
	//zdx(x) = 1/(2 * dx) * (z(x + dx) - z(x - dx))
	//(and similarly for y)
	float zdxp = texture(depthTexture, pos + dx).x;
	float zdxn = texture(depthTexture, pos - dx).x;
	float zdx = 0.5 * (zdxp - zdxn);
	//boundry conditions
	zdx = (zdxp == 1.0) ? zc - zdxn : zdx;
	zdx = (zdxn == 1.0) ? zdxp - zc : zdx;

	//Finite diferences for y
	float zdyp = texture(depthTexture, pos + dy).x;
	float zdyn = texture(depthTexture, pos - dy).x;
	float zdy = 0.5 * (zdyp - zdyn);
	//boundry conditions
	zdy = (zdyp == 1.0) ? zc - zdyn : zdy;
	zdy = (zdyn == 1.0) ? zdyp - zc : zdy;

	//second derivative of the depth
	//Let zdx(x) be the change in depth as above and dx a small change in x, then
	//zdx2(x) can be approximated as:
	//zdx2(x) = 1/(2 * dx) * (zdx(x + dx) - zdx(x - dx))
	//if we chose dx as 0.5 pixels and substitute zdx(x) we get:
	//zdx2(x) = (z(x+1) - z(x)) - (z(x) - z(x-1)) = z(x+1) + z(x-1) - 2 * z(x)
	//This lets us approximate the second derivative without extra texture fetches
	float zdx2 = zdxp + zdxn - 2.0 * zc;
	float zdy2 = zdyp + zdyn - 2.0 * zc;

	//We need this for the derivatives of D
	float zdxpyp = texture(depthTexture, pos + dx + dy).x;
	float zdxnyn = texture(depthTexture, pos - dx - dy).x;
	float zdxpyn = texture(depthTexture, pos + dx - dy).x;
	float zdxnyp = texture(depthTexture, pos - dx + dy).x;
	float zdxy = (zdxpyp + zdxnyn - zdxpyn - zdxnyp) / 4.0;

	//C_x and C_y
	float cx = 2.0 / (viewport.x * -projection[0][0]);
	float cy = 2.0 / (viewport.y * -projection[1][1]);

	//D
	float d = cy * cy * zdx * zdx + cx * cx * zdy * zdy + cx * cx * cy * cy * zc * zc;

	//Derivatives of D
	float ddx = cy * cy * 2.0 * zdx * zdx2 + cx * cx * 2.0 * zdy * zdxy + cx * cx * cy * cy * 2.0 * zc * zdx;
	float ddy = cy * cy * 2.0 * zdx * zdxy + cx * cx * 2.0 * zdy * zdy2 + cx * cx * cy * cy * 2.0 * zc * zdy;

	//E_x and E_y
	float ex = 0.5 * zdx * ddx - zdx2 * d;
	float ey = 0.5 * zdy * ddy - zdy2 * d;

	//Mean Curvature H
	float h = 0.5 * ((cy * ex + cx * ey) / pow(d, 1.5));

	return(vec3(zdx, zdy, h));
}

void main()
{
	//grab the depth from the depthTexture
	float tempDepth = texture(depthTexture, textCoord).x;

	//only smooth the depth if it's not the far plane
	if(tempDepth == 1.0)
		gl_FragDepth = 1.0;
	else
	{
		vec3 dxyz = meanCurvature(textCoord);
		gl_FragDepth = tempDepth + dxyz.z * timeStep * (1.0 + dxyz.x * dxyz.x + dxyz.y * dxyz.y);
	}
}
