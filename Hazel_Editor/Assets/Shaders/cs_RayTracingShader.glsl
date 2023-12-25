//#shader compute
#version 460 core
#define MIN -2147483648
#define MAX 2147483647
#define BG_COLOR vec4(0.0,0.6,0.86,1.0)
#define MAX_NUM_SPHERES 7
#define MAX_RAYS_PER_PIXEL 100

layout (local_size_x = 8, local_size_y = 8) in;

struct LinearBVHNode
{
	int rightChild;
	int triangleStartID;
	int triangleCount;
	float[3] aabbMin;
	float[3] aabbMax;
};

struct RTTriangles
{
	float[3] v0;
	float[3] v1;
	float[3] v2;

	float[2] uv0;
	float[2] uv1;
	float[2] uv2;

	int materialID;
};

uniform layout(binding = 1, rgba8) image2D FinalImage;

layout (std430, binding = 2) buffer layoutLinearBVHNode
{
	LinearBVHNode arrLinearBVHNode[];
}arr_LinearBVHNode;

layout (std430, binding = 3) buffer layoutRTTriangles
{
	RTTriangles arrRTTriangles[];
}arr_RTTriangles;

layout (std430, binding = 4) buffer layoutTriangleIndices
{
	int triIndices[];
}arr_triIndices;

uniform sampler2DArray albedo;

uniform int BVHNodeSize;
uniform int frame_num;
uniform float focal_length;
uniform float time;
uniform vec3 camera_pos;
uniform vec3 camera_viewdir;
uniform vec3 light_dir;
uniform float light_intensity;
uniform int num_bounces;
uniform int samplesPerPixel;

uniform vec4 u_Color;
uniform float u_Roughness;

int num_spheres = MAX_NUM_SPHERES;

uniform mat4 mat_view;
uniform mat4 mat_proj;

uniform int EnvironmentEnabled;
vec4 GroundColour = vec4(0.0);
vec4 SkyColourHorizon = vec4(0.0);
vec4 SkyColourZenith = vec4(0.0);
float SunFocus=20.0;
float SunIntensity=10;

struct Material //sphere material
{
	vec4 color;
	float roughness;
	float metalness;
	vec4 emissive_col;
	float emissive_strength;
};

Material defaultMat;
struct Sphere
{
	vec3 centre;
	float radius;
	Material material;
};

struct Ray
{
	vec3 origin;
	vec3 dir;
};

struct HitInfo
{
	bool isHit;
	float HitDist;
	vec3 HitPos;
	vec3 Normal;
	Material material; //change color to material
};

//random number generator pcg_hash
//https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
float random(inout uint seed) //in 0-1 range
{
    seed = seed * 747796405u + 2891336453u;
    uint word = ((seed >> ((seed >> 28u) + 4u)) ^ seed) * 277803737u;
    word = (word >> 22u) ^ word;
	return word/4294967295.0; //2^32-1
}

float randomNormalDist(inout uint seed)
{
	float theta = 2.0 * 3.1415926 * random(seed);
	float rho = sqrt(-2.0 * log(random(seed)));
	return rho*cos(theta);
}

//random around a sphere
vec3 randomDirInSphere(inout uint seed)
{
	float x = randomNormalDist(seed);
	float y = randomNormalDist(seed);
	float z = randomNormalDist(seed);

	return normalize(vec3(x,y,z));
}


//very handy function to get a sky in shader "taken from Sebastian Lague"
vec4 GetEnvironmentLight(Ray ray)
{
	if(EnvironmentEnabled==0)
		return vec4(0,0,0,1);
	float skyGradientT = pow(smoothstep(0.0, 0.8, ray.dir.y), 0.35);
	float groundToSkyT = smoothstep(-0.01, 0.0, ray.dir.y);
	vec4 skyGradient = mix(SkyColourHorizon, SkyColourZenith, skyGradientT);
	float sun = pow(max(0, dot(ray.dir, -light_dir)), SunFocus) * light_intensity;
	// Combine ground, sky, and sun
	vec4 composite = mix(GroundColour, skyGradient, groundToSkyT) + sun * (groundToSkyT>=1.0?1.0:0.0);
	return composite;
}

//check whether the ray hits the sphere or not
float rayHit(Sphere sh,Ray ray)
{
	vec3 ac = ray.origin - sh.centre;
	float a = dot(ray.dir,ray.dir);
	float b = 2*dot(ray.dir,ac);
	float c = dot(ac,ac)-pow(sh.radius,2);
	float discriminant = b*b - 4*a*c;

	if(discriminant<0)
		return MIN;
	else
		return (-b-sqrt(discriminant))/2*a;
}

HitInfo ClosestHit(Sphere sphere[MAX_NUM_SPHERES],Ray ray)
{
	HitInfo info;
	info.isHit = false; //initially ray will not hit anything
	float min_dist = MAX;
	for(int i=0;i<num_spheres;i++)
	{
		float t = rayHit(sphere[i],ray);			
		if(t >= 0 && t < min_dist) //get hit to nearest sphere
		{
			min_dist = t;
			info.isHit = true; //ray hits a sphere
			info.HitPos = ray.origin + t*ray.dir;
			info.Normal = normalize(info.HitPos - sphere[i].centre);
			info.HitDist = t;
			info.material = sphere[i].material ;
		}			
	}
	return info;
}

//ray-box intersection
bool intersectAABB(vec3 aabbMin,vec3 aabbMax, float t, Ray ray)
{
	float tx1 = (aabbMin.x - ray.origin.x) / ray.dir.x;
	float tx2 = (aabbMax.x - ray.origin.x) / ray.dir.x;
	float tmin = min( tx1, tx2 );
	float tmax = max( tx1, tx2 );
	float ty1 = (aabbMin.y - ray.origin.y) / ray.dir.y;
	float ty2 = (aabbMax.y - ray.origin.y) / ray.dir.y;
	tmin = max( tmin, min( ty1, ty2 ) );
	tmax = min( tmax, max( ty1, ty2 ) );
	float tz1 = (aabbMin.z - ray.origin.z) / ray.dir.z;
	float tz2 = (aabbMax.z - ray.origin.z) / ray.dir.z;
	tmin = max( tmin, min( tz1, tz2 ) ), tmax = min( tmax, max( tz1, tz2 ) );
	return tmax >= tmin && tmin < t && tmax > 0;
}

//ray-triangle intersection
int intersectTriangle(vec3 v0,vec3 v1, vec3 v2,inout float t,inout float u, inout float v, Ray ray)
{
    vec3 edge1 = v1 - v0;
	vec3 edge2 = v2 - v0;
	vec3 h = cross( ray.dir, edge2 );
	float a = dot( edge1, h );
	if (a > -0.0001f && a < 0.0001f) return 0; // ray parallel to triangle
	const float f = 1 / a;
	vec3 s = ray.origin - v0;
	u = f * dot( s, h );
	if (u < 0 || u > 1) return 0;
	vec3 q = cross( s, edge1 );
	v = f * dot( ray.dir, q );
	if (v < 0 || u + v > 1) return 0;
	float p = f * dot( edge2, q );
	if (p > 0.0001f) {t = min( p, t );return 1;}
	return 0;
}

//get closest hit from the bvh
HitInfo ClosestHit(Ray ray)
{
	HitInfo info;
	info.isHit = false; //initially ray will not hit anything
	float t = MAX;
	int i=0;
	int toVisit = 0;
	int nodesToVisit[64];
	float u, v;
	RTTriangles nearestTriangle;//stores the hit triangle info	
	while(true)
	{
		LinearBVHNode node= arr_LinearBVHNode.arrLinearBVHNode[i];
		vec3 aabbMin = vec3(node.aabbMin[0],node.aabbMin[1],node.aabbMin[2]);
		vec3 aabbMax = vec3(node.aabbMax[0],node.aabbMax[1],node.aabbMax[2]);
		if(intersectAABB(aabbMin,aabbMax,t,ray))
		{
			if(node.triangleCount>0)//leaf node
			{
				for(uint k=0; k<node.triangleCount; k++)
				{
					RTTriangles triangle = arr_RTTriangles.arrRTTriangles[arr_triIndices.triIndices[k+node.triangleStartID]];
					vec3 v0 = vec3(triangle.v0[0],triangle.v0[1],triangle.v0[2]);
					vec3 v1 = vec3(triangle.v1[0],triangle.v1[1],triangle.v1[2]);
					vec3 v2 = vec3(triangle.v2[0],triangle.v2[1],triangle.v2[2]);
	
					//get smallest intersection value "t"
					float p = MAX;
					float u_0,v_0;
					if(intersectTriangle(v0, v1, v2, p, u_0, v_0, ray)>0 && p<t)
					{
						t=p;
						u=u_0;v=v_0;
						nearestTriangle=triangle;
					}
				}
				if(toVisit == 0)
					break;
				i = nodesToVisit[--toVisit];	
			}
			else{

					nodesToVisit[toVisit++] = node.rightChild; //put the next child in to the stack
					i=i+1;//left child
			}
		}
		else
		{
			if(toVisit == 0)
				break;
			i = nodesToVisit[--toVisit]; //get the right child		
		}
	}

	info.HitDist = t;
	t!=MAX? info.isHit=true:info.isHit=false;
	info.HitPos = ray.origin + t*ray.dir;
	info.material = defaultMat;
	vec2 uv0 = vec2(nearestTriangle.uv0[0],nearestTriangle.uv0[1]);
	vec2 uv1 = vec2(nearestTriangle.uv1[0],nearestTriangle.uv1[1]);
	vec2 uv2 = vec2(nearestTriangle.uv2[0],nearestTriangle.uv2[1]);
	vec2 uv = uv1*u + uv2*v + uv0*(1.0-u-v);

	info.material.color *= texture(albedo,vec3(uv,nearestTriangle.materialID)); 
	vec3 v0 = vec3(nearestTriangle.v0[0],nearestTriangle.v0[1],nearestTriangle.v0[2]);
	vec3 v1 = vec3(nearestTriangle.v1[0],nearestTriangle.v1[1],nearestTriangle.v1[2]);
	vec3 v2 = vec3(nearestTriangle.v2[0],nearestTriangle.v2[1],nearestTriangle.v2[2]);
	vec3 e01 = v1-v0;
    vec3 e20 = v0-v2;
	info.Normal = normalize(cross(e20,e01));
	return info;
}

//returns color
vec4 perPixel(int numBounces, Sphere sphere[MAX_NUM_SPHERES],Ray ray, inout uint seed)
{
	vec4 color=vec4(1.0);
	vec4 incommingLight = vec4(0);
	for(int k=0;k<=numBounces;k++)
	{
		HitInfo info;
		//info = ClosestHit(sphere,ray);
		info = ClosestHit(ray);		
		if(info.isHit == true)
		{
			ray.origin = info.HitPos + info.Normal*0.0001;
			
			vec3 randomDir = normalize(info.Normal + randomDirInSphere(seed)); //cosine weighted distribution by shifting random direction by the normal
			randomDir = randomDir * sign(dot(info.Normal,randomDir)); //if the ray is inside the hemisphere then invert it
			vec3 DiffuseDir = randomDir;
			vec3 SpecularDir = reflect(ray.dir,info.Normal);			
			ray.dir = mix(DiffuseDir,SpecularDir, (1.0 - info.material.roughness));

			vec4 emittedLight = info.material.emissive_col * info.material.emissive_strength;
			incommingLight += emittedLight*color;
			//color *= mix(info.material.color,vec4(1.0), info.material.metalness);
			color *= info.material.color;
		}
		else{
			incommingLight += GetEnvironmentLight(ray)*color; //get the environment color
			break;
		}
	}
	return incommingLight;
}

vec4 ColorCorrection(vec4 color)
{
	color = clamp(color,0,1);
	color = pow(color, vec4(1.0/2.2)); //Gamma space

	//color = clamp(color,0,1);
	color = vec4(1.0) - exp(-color * 1);//exposure

	//color = clamp(color,0,1);
	//color = mix(vec3(dot(color,vec3(0.299,0.587,0.114))), color,1.0);//saturation

	//color = clamp(color,0,1);
	//color = 1.00*(color-0.5) + 0.5 + 0.00 ; //contrast

	return color;
}

void main()
{
	vec4 color = vec4(0); //background color
	ivec2 uv = ivec2(gl_GlobalInvocationID.xy);

	vec2 image_res = gl_NumWorkGroups.xy*gl_WorkGroupSize.xy;
	vec2 coord = gl_GlobalInvocationID.xy/image_res;
	coord = coord*2.0-1.0;
	vec4 target = inverse(mat_proj) * vec4(coord,1,1); //to view space from clip-space
	
	Ray ray;
	ray.origin = camera_pos;//in world space
	ray.dir = normalize(vec3(inverse(mat_view)*vec4(target.xyz/target.w,0))); //ray dir in world space

	Material mat[MAX_NUM_SPHERES];
	mat[0].color =  u_Color;
	mat[0].roughness = u_Roughness;
	mat[0].metalness = 0.0;
	mat[0].emissive_col = vec4(0.9,0.67,0.1,1.0);
	mat[0].emissive_strength = 0.0;

	mat[1].color =  vec4(0,0,0,1.0);
	mat[1].emissive_col = vec4(0.9,0.67,0.1,1.0);
	mat[1].roughness = 0.13;
	mat[1].metalness = 0.0;
	mat[1].emissive_strength = 15.0;

	mat[2].color =  vec4(0.8,0.2,0.5,1.0);
	mat[2].roughness = 0.98;
	mat[2].metalness = 0.0;
	mat[2].emissive_col = vec4(0.9,0.67,0.1,1.0);
	mat[2].emissive_strength = 0.0;

	mat[3].color =  vec4(0.0,0.514,1.0,1.0);
	mat[3].roughness = 0.68;
	mat[3].metalness = 1.0;
	mat[3].emissive_col = vec4(0.9,0.67,0.1,1.0);
	mat[3].emissive_strength = 0.0;

	mat[4].color =  vec4(1.0,1.0,1.0,1.0);
	mat[4].roughness = 0.13;
	mat[4].metalness = 1.0;
	mat[4].emissive_col = vec4(0.9,0.67,0.1,1.0);
	mat[4].emissive_strength = 0.0;

	mat[5].color =  vec4(1.0,0.518,0.0,1.0);
	mat[5].roughness = 1.0;
	mat[5].metalness = 0.0;
	mat[5].emissive_col = vec4(1.0,0.518,0.0,1.0);
	mat[5].emissive_strength = 0.0;

	mat[6].color =  vec4(0.822,1.0,0.278,1.0);
	mat[6].roughness = 0.2;
	mat[6].metalness = 1.0;
	mat[6].emissive_col = vec4(1.0,0.518,0.0,1.0);
	mat[6].emissive_strength = 0.0;

	Sphere sphere[MAX_NUM_SPHERES];
	sphere[0].centre = vec3(0,-810,0);
	sphere[0].radius = 800.0;	

	sphere[1].centre = vec3(0,-6,430);
	sphere[1].radius = 200.0;

	sphere[2].centre = vec3(18,0,5);
	sphere[2].radius = 10.0;

	sphere[3].centre = vec3(-18,0,5);
	sphere[3].radius = 10.0;

	sphere[4].centre = vec3(0,0,25);
	sphere[4].radius = 10.0;

	sphere[5].centre = vec3(30,0,-15);
	sphere[5].radius = 10.0;

	sphere[6].centre = vec3(-30,0,-25);
	sphere[6].radius = 10.0;

	for(int i=0;i<MAX_NUM_SPHERES;i++)
		sphere[i].material = mat[i];

	defaultMat = mat[0];//for now do this 

	uint hash = uint(uv.x + image_res.x * uv.y + frame_num* 1874);
	for(int i=0; i<samplesPerPixel; i++)
		color += perPixel(num_bounces, sphere, ray, hash); //10 bounces per ray
	color /= samplesPerPixel;

	//progressive render
	float weight = 1.0/(frame_num+1.0);
	color = color * weight + imageLoad(FinalImage,uv)*(1.0-weight);
	//color = clamp(ColorCorrection(color),0.0,1.0);
	imageStore(FinalImage,uv,color);
}