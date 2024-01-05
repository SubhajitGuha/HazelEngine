//#shader compute
#version 460 core
#define SHADOW 1
#define MIN -2147483648
#define MAX 2147483647
#define BG_COLOR vec4(0.0,0.6,0.86,1.0)
#define MAX_NUM_SPHERES 7
#define MAX_RAYS_PER_PIXEL 100
#define PI 3.14159265359

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
vec4 GroundColour = vec4(0.664f, 0.887f, 1.000f, 1.000f);
vec4 SkyColourHorizon = vec4(1.0,1.0,1.0,1.0);
vec4 SkyColourZenith = vec4(0.1,0.4,0.89,1.0);
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
    seed = ((seed >> ((seed >> 28u) + 4u)) ^ seed) * 277803737u;
    seed = (seed >> 22u) ^ seed;
	return seed/4294967295.0; //2^32-1
}

float randomNormalDist(inout uint seed)
{
	float theta = 2.0 * 3.1415926 * random(seed);
	float rho = sqrt(-2.0 * log(random(seed)));
	return rho*cos(theta);
}

//random around a sphere
vec3 randomDirInSphere(vec3 normal,inout uint seed,float alpha)
{
	//float x = randomNormalDist(seed);
	//float y = randomNormalDist(seed);
	//float z = randomNormalDist(seed);

	float r1 = random(seed);
    float r2 = random(seed);
    float phi = 2*PI*r1;
	float sqrt_r2 = sqrt(r2);
    float x = cos(phi)*sqrt_r2;
    float y = sin(phi)*sqrt_r2;
    float z = sqrt(1-r2);
	//float cosTheta = pow(random(seed), 1.0f / (alpha + 1.0f));
    //float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
    //float phi = 2.0f * PI * random(seed);
    vec3 tangentSpaceDir = vec3(x,y,z);

	vec3 up = vec3(1.0, 0.0, 0.0);
	if(abs(normal.x)>0.99f)
		up = vec3(0.0, 0.0, 1.0);
	vec3 tangent = normalize(cross(normal,up));
	vec3 bitangent = cross(normal,tangent);

	return normalize(mat3(tangent,bitangent,normal)*tangentSpaceDir);
}

vec3 ImportanceSamplingGGX(inout uint seed,vec3 N, float roughness)
{
	float a2 = roughness*roughness;
	float r1 = random(seed);
	float r2 = random(seed);
	//GGX sampling see https://schuttejoe.github.io/post/ggximportancesamplingpart1/
	float phi = 2.0 * PI * r1;
    float cosTheta = sqrt((1.0 - r2) / (1.0 + (a2 * a2 - 1.0) * r2));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	//from spherical coordinates to cartesian coordinates
	vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

	// from tangent-space vector to world-space sample vector
    vec3 up = abs(N.x) > 0.99f ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

	vec3 halfVec = mat3(tangent,bitangent,N) * H; //to world space from tangent-space
    halfVec = normalize(halfVec);
    
    return halfVec;
}

float ImportanceSampleGGX_PDF(float NDF, float NdotH, float VdotH) //for specular
{
      //ImportanceSampleGGX pdf
    return NDF * NdotH / (4.0f * VdotH); //NDF is the Dggx version
}

float CosinSamplingPDF(float NdotL) //for diffuse
{
    return NdotL / PI;
}

//very handy function to get a sky in shader "taken from Sebastian Lague"
vec3 GetEnvironmentLight(Ray ray)
{
	if(EnvironmentEnabled==0)
		return vec3(0.0f);
	float skyGradientT = pow(smoothstep(0.0, 0.8, ray.dir.y), 0.35);
	float groundToSkyT = smoothstep(-0.01, 0.0, ray.dir.y);
	vec3 skyGradient = mix(SkyColourHorizon.rgb, SkyColourZenith.rgb, skyGradientT);
	float sun = pow(max(0, dot(ray.dir, -light_dir)), SunFocus) * light_intensity;
	// Combine ground, sky, and sun
	vec3 composite = mix(GroundColour.rgb, skyGradient, groundToSkyT) + sun * (groundToSkyT>=1.0?1.0:0.0);
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
				if(toVisit == 0) //if the stack is empty then break
					break;
				i = nodesToVisit[--toVisit]; //get the next child ("right child") from stack top.	
			}
			else
			{
				nodesToVisit[toVisit++] = node.rightChild; //put the next child ("right child") in to the stack
				i=i+1;//left child
			}
		}
		else
		{
			if(toVisit == 0)
				break;
			i = nodesToVisit[--toVisit]; //get the next child node (right child)
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

	info.material.color = texture(albedo,vec3(uv,nearestTriangle.materialID)); 
	vec3 v0 = vec3(nearestTriangle.v0[0],nearestTriangle.v0[1],nearestTriangle.v0[2]);
	vec3 v1 = vec3(nearestTriangle.v1[0],nearestTriangle.v1[1],nearestTriangle.v1[2]);
	vec3 v2 = vec3(nearestTriangle.v2[0],nearestTriangle.v2[1],nearestTriangle.v2[2]);
	vec3 e01 = v1-v0;
    vec3 e20 = v0-v2;
	info.Normal = normalize(cross(e20,e01));
	return info;
}

float alpha = 0.0;
vec3 F0 = vec3(0.04);
float NormalDistribution_GGX(float NdotH)
{
	float alpha2 =  pow(alpha,4); // alpha is actually the Roughness
	return alpha2 / (PI * pow( (pow(NdotH,2) * (alpha2 - 1.0) + 1.0) ,2) ) ;
}

float Geometry_GGX(float dp) //dp = Dot Product
{
	float k = pow(alpha+1,2) / 8.0;
	return dp/(dp * (1-k) + k);
}

vec3 Fresnel(float VdotH)
{	
	return F0 + (vec3(1.0) - F0) * pow(1.0 - VdotH, 5.0);
}

vec3 FresnelSchlickRoughness(float VdotH, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - VdotH, 5.0);
}

vec3 SpecularBRDF(float Dggx, float Gggx, vec3 fresnel, float NdotV, float NdotL, vec3 Normal)
{
	float denominator = 4.0 * NdotL * NdotV;
	vec3 specular = (Dggx * Gggx * fresnel) / denominator;
	return specular;
}

//returns color
vec3 perPixel(int numBounces, Ray ray,Sphere[MAX_NUM_SPHERES] sphere, inout uint seed)
{
	vec3 color=vec3(1.0);
	vec3 incommingLight = vec3(0.0);
	for(int k=0;k<numBounces;k++)
	{
		HitInfo info;
		info = ClosestHit(sphere,ray);
		//info = ClosestHit(ray);		
		if(info.isHit == true)
		{
			alpha = info.material.roughness;
			float roulette = random(seed);
			vec3 reflectionDir;
			float diffuseRatio = 0.5f * (1.0f - info.material.metalness);
			float specularRatio = 1.0f - diffuseRatio;
			vec3 V = -ray.dir;
			
			//determine the direction of the next ray
			if(roulette < diffuseRatio)
			{//sample diffuse lobe
				reflectionDir = randomDirInSphere(info.Normal,seed,1.0f); //cosine weighed hemispherical sampling
			}
			else
			{//sample specular lobe
				vec3 halfVec = ImportanceSamplingGGX(seed, info.Normal, alpha);
				reflectionDir = -normalize(reflect(V,halfVec));
			}

			vec3 L = reflectionDir;
			vec3 H = normalize(V+L);
			float NdotL = max(dot(info.Normal, L),0.001);
            float NdotH = max(dot(info.Normal, H),0.001);
            float VdotH = max(dot(V, H),0.001);
			float NdotV = max(dot(info.Normal,V),0.001);

			if(info.material.metalness>0)
				F0 = vec3(0.8f);
			else
				F0 = vec3(0.04f);
			float specular = 1.0;
			F0 = mix(F0*specular, info.material.color.rgb, info.material.metalness);
		
			float Dggx = NormalDistribution_GGX(NdotH);
			float Gggx = Geometry_GGX(NdotV) * Geometry_GGX(NdotL);
			vec3 fresnel = Fresnel(VdotH);
			vec3 ks = fresnel;
			vec3 kd = vec3(1.0)-ks;
			kd *= (1.0f - info.material.metalness);

			vec3 specularBRDF = SpecularBRDF(Dggx, Gggx, fresnel, NdotV, NdotL, info.Normal);
			vec3 diffuseBRDF = info.material.color.rgb / PI;
			float specularPDF = ImportanceSampleGGX_PDF(Dggx,NdotH,VdotH);
			float diffusePDF = CosinSamplingPDF(NdotL);

			vec3 totalBRDF = (kd * diffuseBRDF + specularBRDF) * NdotL;
			float totalPDF = diffusePDF*diffuseRatio + specularPDF*specularRatio;

			ray.origin = info.HitPos + info.Normal*0.001;
			ray.dir = reflectionDir;

			vec3 emittedLight = info.material.emissive_col.rgb * info.material.emissive_strength;
			incommingLight += emittedLight*color;
			if(totalPDF > 0.0)
				color *= totalBRDF/totalPDF;
		}
		else{
			incommingLight += GetEnvironmentLight(ray)*color; //get the environment color
			break;
		}
	}
	////calculate shadow	
	//ray.dir = -light_dir;
	//HitInfo info = ClosestHit(ray);
	//if(info.isHit == true)
	//{
	//	incommingLight *= vec4(0.2,0.2,0.2,1.0);//multiplying incomming light with shadow color
	//}
	return incommingLight;
}

vec3 ColorCorrection(vec3 color)
{
	//color = clamp(color,0,1);
	color = pow(color, vec3(1.0/2.2)); //Gamma space

	//color = clamp(color,0,1);
	color = vec3(1.0) - exp(-color * 2);//exposure

	//color = clamp(color,0,1);
	//color = mix(vec3(dot(color,vec3(0.299,0.587,0.114))), color,1.0);//saturation

	//color = clamp(color,0,1);
	//color = 1.00*(color-0.5) + 0.5 + 0.00 ; //contrast

	return color;
}

void main()
{
	vec3 color = vec3(0.0); //background color
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
	mat[0].metalness = 1.0;
	mat[0].emissive_col = vec4(0.9,0.67,0.1,1.0);
	mat[0].emissive_strength = 0.0;

	mat[1].color =  vec4(1.0,0.67,0.1,1.0);
	mat[1].emissive_col = vec4(1.0,0.67,0.1,1.0);
	mat[1].roughness = 0.13;
	mat[1].metalness = 0.0;
	mat[1].emissive_strength = 0.0;

	mat[2].color =  vec4(0.8,0.2,0.5,1.0);
	mat[2].roughness = 0.08;
	mat[2].metalness = 0.0;
	mat[2].emissive_col = vec4(0.9,0.67,0.1,1.0);
	mat[2].emissive_strength = 0.0;

	mat[3].color =  vec4(0.0,0.514,1.0,1.0);
	mat[3].roughness = 0.68;
	mat[3].metalness = 1.0;
	mat[3].emissive_col = vec4(0.9,0.67,0.1,1.0);
	mat[3].emissive_strength = 0.0;

	mat[4].color =  vec4(1.0,1.0,1.0,1.0);
	mat[4].roughness = 0.10;
	mat[4].metalness = 1.0;
	mat[4].emissive_col = vec4(0.9,0.67,0.1,1.0);
	mat[4].emissive_strength = 0.0;

	mat[5].color =  vec4(1.0,0.518,0.0,1.0);
	mat[5].roughness = 0.1;
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

	sphere[1].centre = vec3(0,-6,0);
	sphere[1].radius = 5.0;

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


	defaultMat = mat[0];//for now do this 

	uint hash = uint(uv.x + image_res.x * uv.y + frame_num* 1874);
	for(int i=0; i<samplesPerPixel; i++)
		color += perPixel(num_bounces, ray, sphere, hash); //10 bounces per ray
	color /= samplesPerPixel;

	//progressive render
	float weight = 1.0/(frame_num);
	color = color * weight + imageLoad(FinalImage,uv).rgb*(1.0-weight);
	//color = clamp(ColorCorrection(color),0.0,1.0);
	imageStore(FinalImage,uv,vec4(color,1.0));
}