#version 150 core
#extension GL_ARB_explicit_attrib_location : require

in vec3 v_sky;

layout (location = 0) out vec4 color;

uniform samplerCube skybox;
uniform vec4 metaballs[20];
uniform vec3 eye;
uniform mat4 projection;
uniform mat4 view;

const float INFINITY = 1e+4;
const float EPSILON  = 1e-6;

const int SIZE = 20;
const float TRESHOLD = 0.33;
const vec3 light = normalize(vec3(1.0, 0.0, -1.0));
const vec3 ambient = vec3(0.3725, 0.3686, 0.4314);

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

struct Material
{
	vec4 sr; // vec3 specular, float reflectance
	vec4 dr; // vec3 diffuse, float roughness
};


/* COOKTORRANCE.FRAG */
// Schlick's Approximation of the Fresnel Factor
float fresnel(in float VdotH, in float r)
{
	// r: reflectance
	return pow(1.0 - VdotH, 5) * (1 - r) + r;
}

// Beckmann's distribution for roughness
float roughness(in float NdotH, in float r)
{
	// r: roughness

	float r2 = r * r;
	float a = 1.0 / (4.0 * r2  * pow(NdotH, 4.0));
	float b = NdotH * NdotH - 1.0;
	float c = r2 * NdotH * NdotH;

	return a * exp(b / c);
}

// Geometric attenuation accounts for the shadowing and 
// self masking of one microfacet by another.
float geom(in float NdotH, in float NdotV, in float VdotH, in float NdotL)
{
	float geo_numerator   = 2.0 * NdotH;
    float geo_denominator = 1.0 / VdotH;

    float a = (geo_numerator * NdotV) * geo_denominator;
    float b = (geo_numerator * NdotL) * geo_denominator;
    return min(1.0, min(a, b));
}

vec3 CookTorrance(in vec3 V, in vec3 N, in vec3 L/*, in Material m*/, in vec3 R, in vec3 ambient)
{
	vec3 H = normalize(L + V);

	float VdotH = clamp(dot(V, H), 0.0, 1.0);
	float NdotV = clamp(dot(N, V), 0.0, 1.0);
	float NdotH = clamp(dot(N, H), 0.0, 1.0);
	float NdotL = clamp(dot(N, L), 0.0, 1.0);

	float Rs = clamp(geom(NdotH, NdotV, VdotH, NdotL) * fresnel(VdotH, 0.44) * roughness(NdotH, 0.15), 0.0, 1.0); 
	Rs /= clamp(NdotV * NdotL, EPSILON, 1.0);

	float r2 = 0.3 * 0.3;

	vec3 c = mix(vec3(0.1, 0.1, 0.05) * Rs + vec3(0.05, 0.05, 0.05), R, r2);
	
	return mix(mix(ambient, R, r2) * c, c, NdotL);
}


/* METABALLS.FRAG */
bool intersect(in vec4 blob, const in Ray ray, out float t0, out float t1)
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

float sum(in vec3 pos)
{
	float s = 0.0;
	for(int i = 0; i < SIZE; ++i)
	{
		Sphere blob;
		blob.position = metaballs[i].xyz;
		blob.radius = metaballs[i].w;
		s += smoothstep(blob.radius, 0.0, distance(pos, blob.position));
	}
	return s;
}

void interp(in vec3 pos, out vec3 N/*, out Material M*/)
{
	float 	 W = 0;
	N 		   = vec3(0.0);
	//M.KasdIOR  = vec4(0.0);
	//M.sr       = vec4(0.0);
	//M.dr       = vec4(0.0);

	for(int i = 0; i < SIZE; ++i)
	{
		Sphere blob;
		blob.position = metaballs[i].xyz;
		blob.radius = metaballs[i].w;
		//Material m  = materials[i];
		
		float w = smoothstep(blob.radius, 0.0, distance(pos, blob.position));

		N += normalize(pos - blob.position) * w;
		W += w;
		
		//M.KasdIOR += m.KasdIOR * w;
		//M.sr 	  += m.sr * w;
		//M.dr 	  += m.dr * w;
	}
	N = normalize(N);
	W = 1.0 / W;
	//M.sr *= W;
	//M.dr *= W;
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

bool trace(in Ray ray, out vec3 normal/*, out Material material*/, out float t)
{		
	float tmin =  INFINITY;
	float tmax = -INFINITY;
	
	for(int i = 0; i < SIZE; ++i)
	{
		float t0 = INFINITY;
		float t1 = INFINITY;
		
		if(intersect(metaballs[i], ray, t0, t1))
		{
			tmin = min(t0, tmin);
			tmax = max(t1, tmax);
		}
	}
	t = max(tmin, 0.2);
	vec3 pos;
	float delta[2]; 
	delta[0] = 1.0;
	delta[1] = 1.0;

	float i = 0;
	
	while(t < tmax)
	{
		pos = ray.origin + ray.direction * t;
		float s = sum(pos);
		
		delta[1] = delta[0];
		delta[0] = TRESHOLD - s;
		
		i += 0.25;
		if(i > 64 || abs(delta[0]) < 0.02)
		{
			interp(pos, normal/*, material*/);
			return true;
		}

		t += delta[0] * 0.33;
	}
	return false; 
}

void main()
{
	Ray ray[3];
	vec3 h[2];

	// initial ray
	ray[0].origin    = eye;
	ray[0].direction = normalize(v_sky);
	
	vec3 	 l = normalize(light); // -ray[0].direction; 
	vec3     n[2];
	//Material m[2];

	float t;
	vec3 c;
	
	float depth = 1.0;
	
	float lum = 0.0;
	
	if(trace(ray[0], n[0]/*, m[0]*/, t))
	{
		vec3 v[2];

		ray[1].origin = ray[0].origin + ray[0].direction * t;
		ray[1].direction = reflect(ray[0].direction, n[0]);
		v[0] = -ray[0].direction;
		vec3 R;
		
		vec4 frpos = projection * view * vec4(ray[1].origin, 1.0);
		depth = frpos.z / frpos.w;

		if(trace(ray[1], n[1]/*, m[1]*/, t))
		{
			ray[2].origin = ray[1].origin + ray[1].direction * t;
			ray[2].direction = reflect(ray[1].direction, n[1]);
			v[1] = -ray[1].direction;

			R = CookTorrance(v[1], n[1], l/*, m[1]*/, texture(skybox, vec3(ray[2].direction.x, -ray[2].direction.y, ray[2].direction.z)).xyz, ambient);			
		}
		else
			R = texture(skybox, vec3(ray[1].direction.x, -ray[1].direction.y, ray[1].direction.z)).xyz;

		c = CookTorrance(v[0], n[0], l/*, m[0]*/, R, ambient);
	}
	else
	{
		c = texture(skybox, vec3(ray[0].direction.x, -ray[0].direction.y, ray[0].direction.z)).xyz;
		lum = (c.r * 11.0 + c.g * 16.0 + c.b * 5.0) / 32.0;
	}	
	lum = (c.r * 11.0 + c.g * 16.0 + c.b * 5.0) / 32.0;
	color = vec4(c, smoothstep(0.92, 1.0, lum * 1.0));
	gl_FragDepth = depth;
}