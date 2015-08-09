#version 330 core

//makes circles out of the planes and sets verex depths to simulate spheres

uniform mat4 projection;

in vec2 textcoord;
in vec3 viewSpacePos;
in float metaRadius;

void main()
{
    //calculate the coordinate on the plane through the texture coordinates
    vec2 position = textcoord * 2.0 - vec2(1.0);

    //the square distance from the center of the plane
    float sqRadius = dot(position, position);

    //discard fragments outside the circle
    if (sqRadius > 1.0) discard;

    //calculate the distance the fragment needs to be moved towards the camera
    //with pythagoras
    float radiusOffset = sqrt(1.0 - sqRadius);

    //set fragment position and depth
    vec4 fragmentPos = vec4(viewSpacePos.xy, viewSpacePos.z + radiusOffset * metaRadius, 1.0);
    vec4 clipSpacePos = projection * fragmentPos;
    gl_FragDepth = clipSpacePos.z / clipSpacePos.w;
}
