#version 150

in vec3 v_sky;

out vec4 color;

uniform vec4 metaballs[16];
uniform vec3 eye;

const float INFINITY = 1e+4;
const int SIZE = 16;

struct Sphere
{
	vec3 position;
	float radius;
};

struct Ray
{
	vec3 origin;
	vec3 direction;
};


bool intersect(
    in vec4 blob
,   const in Ray ray // needs to be normalized!
,   out float t0
,   out float t1)
{
	vec3  d = ray.origin - blob.xyz;

	float b = 2 * dot(ray.direction, d);
	float c = dot(d, d) - blob.w * blob.w;

	float t = b * b - 4 * c;
	if(t > 0.0)
	{
		t0 = 0.5 * (-b - sqrt(t));
		t1 = 0.5 * (-b + sqrt(t));
		return true;
	}	
	return false;
}


bool rcast(in Ray ray, out vec3 normal, out float t)
{	

	normal = vec3(0.0);

	t =  INFINITY;
	
	for(int i = 0; i < SIZE; ++i)
	{
		float t0 = INFINITY;
		float t1 = INFINITY;
		
		if(intersect(metaballs[i], ray, t0, t1) && t0 < t && t0 > 0.0)
		{
			t = t0;
			
			normal = normalize((ray.origin + ray.direction * t) - metaballs[i].xyz);
		}
	}
	return t > 0.0 && t < INFINITY;
}


void main()
{
	Ray ray;

	// initial ray
	ray.origin    = eye;
	ray.direction = normalize(v_sky);
	
	vec3 n;
	float t;
	
	vec3 c;

<<<<<<< HEAD
	rcast(ray, n, t);	
=======
	rcast(ray, n, t);
>>>>>>> master
	c = n * 0.5 + 0.5;
	
	color = vec4(c, 1.0);
}
