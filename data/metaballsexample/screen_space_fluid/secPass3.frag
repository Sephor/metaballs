#version 330

uniform sampler2D depthTexture;

uniform vec2 viewport;
uniform float binomCoeff[30];

in vec2 textCoord;

float calcDepth(float originDepth, ivec2 uv)
{
    int coeffSize = 29;
	vec2 dx = vec2(1.0 / viewport.x, 0.0);

    float sum = originDepth * binomCoeff[0];

    for (int i=1; i<=coeffSize; ++i)
    {
        float rightDepth = texture(depthTexture, vec2(uv / viewport) + i * dx).x;
        float leftDepth = texture(depthTexture, vec2(uv / viewport) - i * dx).x;
        sum += binomCoeff[i] *
               (mix(rightDepth, originDepth, step(0.02, abs(rightDepth - originDepth))) +
                mix(leftDepth, originDepth, step(0.02, abs(leftDepth - originDepth))));
    }
    return sum;
}

void main()
{
	ivec2 uv = ivec2(textCoord * vec2(viewport));
    float originDepth = texelFetch(depthTexture, uv, 0).r;

    gl_FragDepth = mix(calcDepth(originDepth, uv), 1.0, step(1.0, originDepth));
}
