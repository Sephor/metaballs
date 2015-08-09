#version 330

//smooth the depthTexture with mean curvature flow

uniform sampler2D depthTexture;
uniform mat4 view;
uniform mat4 projection;
uniform float timeStep;

uniform vec2 viewport;

in vec2 textCoord;

vec3 meanCurvature(vec2 pos) {
	//This implements mean curvature flow as found in the paper "Screen Space
	//Fluid Rendering with Curvature Flow" by Wladimir J. van der Laan,
	//Simon Green, and Miguel Sainz (2009)

	//Width of one pixel
	vec2 dx = vec2(1.0 / viewport.x, 0.0);
	vec2 dy = vec2(0.0, 1.0 / viewport.y);

	//Center depth
	float z =  texture(depthTexture, pos).x;

	//Finite diferences for x
	//let z(x) be the depth at horizontal position x and dx a small enough change
	//in x (e.g. one pixel), then dzdx(x) can be approximated as:
	//dzdx(x) = 1/(2 * dx) * (z(x + dx) - z(x - dx))
	//(and similarly for y)
	float dzdxp = texture(depthTexture, pos + dx).x;
	float dzdxn = texture(depthTexture, pos - dx).x;
	float dzdx = 0.5 * (dzdxp - dzdxn);
	//boundry conditions
	dzdx = (dzdxp == 1.0) ? z - dzdxn : dzdx;
	dzdx = (dzdxn == 1.0) ? dzdxp - z : dzdx;

	//Finite diferences for y
	float dzdyp = texture(depthTexture, pos + dy).x;
	float dzdyn = texture(depthTexture, pos - dy).x;
	float dzdy = 0.5 * (dzdyp - dzdyn);
	//boundry conditions
	dzdy = (dzdyp == 1.0) ? z - dzdyn : dzdy;
	dzdy = (dzdyn == 1.0) ? dzdyp - z : dzdy;

	//second derivative of the depth
	//let dzdx(x) be the change in depth as above and dx a small change in x, then
	//dzdx2(x) can be approximated as:
	//dzdx2(x) = 1/(2 * dx) * (dzdx(x + dx) - dzdx(x - dx))
	//if we chose dx as 0.5 pixels and substitute dzdx(x) we get:
	//dzdx2(x) = (z(x+1) - z(x)) - (z(x) - z(x-1)) = z(x+1) + z(x-1) - 2 * z(x)
	//This lets us approximate the second derivative without extra texture fetches
	float dzdx2 = dzdxp + dzdxn - 2.0 * z;
	float dzdy2 = dzdyp + dzdyn - 2.0 * z;

	//We need this fo the derivatives of D
	float dzdxpyp = texture(depthTexture, pos + dx + dy).x;
	float dzdxnyn = texture(depthTexture, pos - dx - dy).x;
	float dzdxpyn = texture(depthTexture, pos + dx - dy).x;
	float dzdxnyp = texture(depthTexture, pos - dx + dy).x;
	float dzdxy = (dzdxpyp + dzdxnyn - dzdxpyn - dzdxnyp) / 4.0;

	//C_x and C_y
	float cx = 2.0 / (viewport.x * -projection[0][0]);
	float cy = 2.0 / (viewport.y * -projection[1][1]);

	//D
	float d = cy * cy * dzdx * dzdx + cx * cx * dzdy * dzdy + cx * cx * cy * cy * z * z;

	//Derivatives of D
	float ddx = cy * cy * 2.0 * dzdx * dzdx2 + cx * cx * 2.0 * dzdy * dzdxy + cx * cx * cy * cy * 2.0 * z * dzdx;
	float ddy = cy * cy * 2.0 * dzdx * dzdxy + cx * cx * 2.0 * dzdy * dzdy2 + cx * cx * cy * cy * 2.0 * z * dzdy;

	//E_x and E_y
	float ex = 0.5 * dzdx * ddx - dzdx2 * d;
	float ey = 0.5 * dzdy * ddy - dzdy2 * d;

	//Mean Curvature H
	float h = 0.5 * ((cy * ex + cx * ey) / pow(d, 1.5));


	return(vec3(dzdx, dzdy, h));
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
		//as on Wikipedia (https://en.wikipedia.org/wiki/Mean_curvature_flow)
		vec3 dxyz = meanCurvature(textCoord);
		gl_FragDepth = tempDepth + dxyz.z * timeStep * sqrt(1.0 + dxyz.x * dxyz.x + dxyz.y * dxyz.y);
	}
}
