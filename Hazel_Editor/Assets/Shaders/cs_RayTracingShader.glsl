//#shader compute
#version 460 core
#extension GL_ARB_bindless_texture : require //for bindless texture samplers
#extension GL_NV_gpu_shader5 : require //for uint64_t

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

	float[3] n0;
	float[3] n1;
	float[3] n2;

	float[2] uv0;
	float[2] uv1;
	float[2] uv2;

	uint64_t tex_albedo; //bindless albedo texture
	uint64_t tex_roughness; //bindless roughness texture

	int materialID;
};

struct Material
{
	float[4] color;
	float roughness;
	float metalness;
	float[4] emissive_col;
	float emissive_strength;
};

uniform layout(binding = 1, rgba16f) image2D FinalImage;

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

layout (std430, binding = 5) buffer layoutMaterials
{
	Material arrMaterials[];
}arr_Materials;

uniform sampler2DArray albedo;
uniform sampler2DArray roughness_metallic;

uniform int BVHNodeSize;
uniform int frame_num;
uniform int sample_count;
uniform float focal_length;
uniform float time;
uniform vec3 camera_pos;
uniform vec3 camera_viewdir;
uniform vec3 light_dir;
uniform float light_intensity;
uniform int num_bounces;
uniform int samplesPerPixel;

uniform vec3 LightPos;
uniform float u_LightStrength;

int num_spheres = MAX_NUM_SPHERES;

uniform mat4 mat_view;
uniform mat4 mat_proj;

uniform int EnvironmentEnabled;

uniform int TileIndex_X;
uniform int TileIndex_Y;
uniform float u_ImageWidth;
uniform float u_ImageHeight;

vec4 GroundColour = vec4(0.664f, 0.887f, 1.000f, 1.000f);
vec4 SkyColourHorizon = vec4(1.0,1.0,1.0,1.0);
vec4 SkyColourZenith = vec4(0.1,0.4,0.89,1.0);
float SunFocus=20.0;
float SunIntensity=10;

int numLights = 1; //needs to be an uniform

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
	bool isEmitter;
	Material material; //change color to material
};

struct Light
{
	vec3 emission_color;
	float emission_strength;
	vec3 position;   
    vec3 u;
    vec3 v;
    //float type;
};
Light lights[1];

struct LightInfo
{
	vec3 emission_color;
	vec3 normal;
	float emission_strength;
	float area;
	vec3 direction;
	float LightDist;
	float pdf;
};

struct BRDFInformation
{
	vec3 totalBRDF;
	float brdfPDF;
	vec3 reflectionDir;

};
vec4 float4_to_vec4(float[4] arr)
{
	return vec4(arr[0],arr[1],arr[2],arr[3]);
}

//random number generator pcg_hash
//https://www.shadertoy.com/view/wltcRS
uvec4 seed;
void pcg4d(inout uvec4 v)
{
    v = v * 1664525u + 1013904223u;
    v.x += v.y * v.w; v.y += v.z * v.x; v.z += v.x * v.y; v.w += v.y * v.z;
    v = v ^ (v >> 16u);
    v.x += v.y * v.w; v.y += v.z * v.x; v.z += v.x * v.y; v.w += v.y * v.z;
}

float random() //in 0-1 range
{
	 pcg4d(seed); return float(seed.x) / float(0xffffffffu);
}

//random around a sphere
vec3 randomDirInSphere(vec3 normal,float alpha)
{
	//float r1 = random();
    //float r2 = random();
    //float phi = 2*PI*r1;
	//float sqrt_r2 = sqrt(r2);
    //float x = cos(phi)*sqrt_r2;
    //float y = sin(phi)*sqrt_r2;
    //float z = sqrt(1-r2);
	float cosTheta = pow(random(), 1.0f / (alpha + 1.0f));
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
    float phi = 2.0f * PI * random();
    vec3 tangentSpaceDir = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

	vec3 up = vec3(1.0, 0.0, 0.0);
	if(abs(normal.x)>0.99f)
		up = vec3(0.0, 0.0, 1.0);
	vec3 tangent = normalize(cross(normal,up));
	vec3 bitangent = cross(normal,tangent);

	return normalize(mat3(tangent,bitangent,normal)*tangentSpaceDir);
}

vec3 ImportanceSamplingGGX(vec3 N, float roughness)
{
	float a2 = roughness*roughness;
	float r1 = random();
	float r2 = random();
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
float intersectSphere(Sphere sh,Ray ray)
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
		float t = intersectSphere(sphere[i],ray);			
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

float intersectQuad(in Ray ray, in vec3 u, in vec3 v, in vec3 normal, in vec3 QuadPos)
{
	float t;
	float denom = dot(ray.dir,normal);	
	t = dot((QuadPos-ray.origin),normal)/denom;
	if(t > 0.0003f)
	{
		vec3 pointInPlane = ray.origin + t*ray.dir;
		vec3 vi = (pointInPlane - QuadPos);
		float a1 = dot(vi,normalize(u));
		if(a1>=0.0f && a1<= length(u))
		{
			float a2 = dot(vi,normalize(v));
			if(a2>=0.0f && a2<= length(v))
			{				
				return t;
			}
		}
	}

	return MAX;
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

bool AnyHit(Ray ray, float range)
{
	HitInfo info;
	info.isHit = false; //initially ray will not hit anything
	float t = range;
	int i=0;
	int toVisit = 0;
	int nodesToVisit[64];
	float u, v;
	RTTriangles nearestTriangle;//stores the hit triangle info

	for(int k=0; k<numLights; k++)
	{
		Light light = lights[k];
		vec3 light_normal = cross(light.u,light.v);
		float light_area = length(light_normal);
		light_normal /= light_area;
		if (dot(light_normal, ray.dir) > 0.) // Hide backfacing quad light
                continue;

		float p = intersectQuad(ray, light.u, light.v, light_normal, light.position);
		if(p<t)
		{
			return true;		
		}
	}

	while(true)
	{
		LinearBVHNode node= arr_LinearBVHNode.arrLinearBVHNode[i]; //LinearBVHNode is stored in DFS manner
		vec3 aabbMin = vec3(node.aabbMin[0],node.aabbMin[1],node.aabbMin[2]);
		vec3 aabbMax = vec3(node.aabbMax[0],node.aabbMax[1],node.aabbMax[2]);
		if(intersectAABB(aabbMin,aabbMax,t,ray))
		{
			if(node.triangleCount>0)//leaf node
			{
				for(uint k=0; k<node.triangleCount; k++) //iterate through all the triangles in the leaf node
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
						return true; //intersection is found
					}
				}
				if(toVisit == 0) //if the stack is empty then break
					break;
				i = nodesToVisit[--toVisit]; //get the next child ("right child") from stack top.	
			}
			else
			{
				nodesToVisit[toVisit++] = node.rightChild; //put the next child ("right child") in to the stack
				i=i+1;//left child as LinearBVHNode is stored in DFS manner
			}
		}
		else
		{
			if(toVisit == 0)
				break;
			i = nodesToVisit[--toVisit]; //get the next child node (right child)
		}
	}
	return false;
}

//get closest hit from the bvh
HitInfo ClosestHit(Ray ray, inout LightInfo light_info)
{
	HitInfo info;
	info.isHit = false; //initially ray will not hit anything
	float t = MAX;
	int i=0;
	int toVisit = 0;
	int nodesToVisit[64];
	float u, v;
	RTTriangles nearestTriangle;//stores the hit triangle info

	for(int k=0; k<numLights; k++)
	{
		Light light = lights[k];
		vec3 light_normal = cross(light.u,light.v);
		float light_area = length(light_normal);
		light_normal /= light_area;
		if (dot(light_normal, ray.dir) > 0.) // Hide backfacing quad light
                continue;

		float p = intersectQuad(ray, light.u, light.v, light_normal, light.position);
		if(p<t)
		{
			t = p;
			info.isEmitter = true;
			light_info.pdf = t*t/(light_area * abs(dot(-ray.dir,light_normal)));
			light_info.emission_color = light.emission_color;
			light_info.emission_strength = light.emission_strength;
		}
	}

	while(true)
	{
		LinearBVHNode node= arr_LinearBVHNode.arrLinearBVHNode[i]; //LinearBVHNode is stored in DFS manner
		vec3 aabbMin = vec3(node.aabbMin[0],node.aabbMin[1],node.aabbMin[2]);
		vec3 aabbMax = vec3(node.aabbMax[0],node.aabbMax[1],node.aabbMax[2]);
		if(intersectAABB(aabbMin,aabbMax,t,ray))
		{
			if(node.triangleCount>0)//leaf node
			{
				for(uint k=0; k<node.triangleCount; k++) //iterate through all the triangles in the leaf node
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
						info.isEmitter = false;
					}
				}
				if(toVisit == 0) //if the stack is empty then break
					break;
				i = nodesToVisit[--toVisit]; //get the next child ("right child") from stack top.	
			}
			else
			{
				nodesToVisit[toVisit++] = node.rightChild; //put the next child ("right child") in to the stack
				i=i+1;//left child as LinearBVHNode is stored in DFS manner
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
	info.material = arr_Materials.arrMaterials[nearestTriangle.materialID];
	vec2 uv0 = vec2(nearestTriangle.uv0[0],nearestTriangle.uv0[1]);
	vec2 uv1 = vec2(nearestTriangle.uv1[0],nearestTriangle.uv1[1]);
	vec2 uv2 = vec2(nearestTriangle.uv2[0],nearestTriangle.uv2[1]);
	vec2 uv = uv1*u + uv2*v + uv0*(1.0-u-v);

	//multiply texture with material color(tint);
	//sampler2D albedo = sampler2D(nearestTriangle.tex_albedo);
	//sampler2D roughness = sampler2D(nearestTriangle.tex_roughness);
	//
	//vec4 tex_albedo = texture(albedo, uv); //load the bindless textures
	//vec4 tex_roughness = texture(roughness, uv);
	//
	//info.material.color[0] *= tex_albedo.r;
	//info.material.color[1] *= tex_albedo.g; 
	//info.material.color[2] *= tex_albedo.b;
	//info.material.color[3] *= tex_albedo.a; 
	//info.material.roughness *= tex_roughness.g;
	////info.material.metalness = tex_roughness.b;

	vec3 n0 = vec3(nearestTriangle.n0[0],nearestTriangle.n0[1],nearestTriangle.n0[2]);
	vec3 n1 = vec3(nearestTriangle.n1[0],nearestTriangle.n1[1],nearestTriangle.n1[2]);
	vec3 n2 = vec3(nearestTriangle.n2[0],nearestTriangle.n2[1],nearestTriangle.n2[2]);

	info.Normal = normalize(n1*u + n2*v + n0*(1.0-u-v));
	//info.Normal = dot(info.Normal,ray.dir)<=0.0? info.Normal : -info.Normal;
	return info;
}

vec3 F0 = vec3(0.04);
float NormalDistribution_GGX(float NdotH,float alpha)
{
	float alpha2 =  pow(alpha,4); // alpha is actually the Roughness
	return alpha2 / (PI * pow( (pow(NdotH,2) * (alpha2 - 1.0) + 1.0) ,2) ) ;
}

float Geometry_GGX(float dp,float alpha) //dp = Dot Product
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

//evaluate cook-torrance BRDF using the specified values of view direction 'V' and light direction 'L'
BRDFInformation EvalBRDF(HitInfo info, vec3 V, vec3 L)
{
	float alpha =  info.material.roughness;
	BRDFInformation brdf_info;
	vec3 reflectionDir;
	float diffuseRatio = 0.5f * (1.0f - info.material.metalness);
	float specularRatio = 1.0f - diffuseRatio;

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
	F0 = mix(F0*specular, float4_to_vec4(info.material.color).rgb, info.material.metalness);
	
	float Dggx = NormalDistribution_GGX(NdotH, alpha);
	float Gggx = Geometry_GGX(NdotV, alpha) * Geometry_GGX(NdotL, alpha);
	vec3 fresnel = Fresnel(VdotH);
	vec3 ks = fresnel;
	vec3 kd = vec3(1.0)-ks;
	kd *= (1.0f - info.material.metalness);

	//pdfs
	float specularPDF = ImportanceSampleGGX_PDF(Dggx,NdotH,VdotH);
	float diffusePDF = CosinSamplingPDF(NdotL);
	brdf_info.brdfPDF = diffusePDF*diffuseRatio + specularPDF*specularRatio;

	//brdfs
	vec3 specularBRDF = SpecularBRDF(Dggx, Gggx, fresnel, NdotV, NdotL, info.Normal);
	vec3 diffuseBRDF = float4_to_vec4(info.material.color).rgb / PI;			
	brdf_info.totalBRDF = (kd * diffuseBRDF + specularBRDF) * NdotL;

	return brdf_info;
}

//evaluate cook-torrance BRDF and sampling directions using Importance Sampling
BRDFInformation CalcBRDF(HitInfo info, vec3 V)
{
	float alpha =  info.material.roughness;
	BRDFInformation brdf_info;
	float roulette = random();
	vec3 reflectionDir;
	float diffuseRatio = 0.5f * (1.0f - info.material.metalness);
	float specularRatio = 1.0f - diffuseRatio;
	
	//determine the direction of the next ray
	if(roulette < diffuseRatio)
	{//sample diffuse lobe
		reflectionDir = randomDirInSphere(info.Normal,1.0f); //cosine weighed hemispherical sampling
	}
	else
	{//sample specular lobe
		vec3 halfVec = ImportanceSamplingGGX(info.Normal, alpha);
		reflectionDir = -normalize(reflect(V,halfVec));
	}
	
	brdf_info.reflectionDir = reflectionDir;
	vec3 L = brdf_info.reflectionDir;
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
	F0 = mix(F0*specular, float4_to_vec4(info.material.color).rgb, info.material.metalness);
	
	float Dggx = NormalDistribution_GGX(NdotH, alpha);
	float Gggx = Geometry_GGX(NdotV, alpha) * Geometry_GGX(NdotL, alpha);
	vec3 fresnel = Fresnel(VdotH);
	vec3 ks = fresnel;
	vec3 kd = vec3(1.0)-ks;
	kd *= (1.0f - info.material.metalness);

	//pdfs
	float specularPDF = ImportanceSampleGGX_PDF(Dggx,NdotH,VdotH);
	float diffusePDF = CosinSamplingPDF(NdotL);
	brdf_info.brdfPDF = diffusePDF*diffuseRatio + specularPDF*specularRatio;

	//brdfs
	vec3 specularBRDF = SpecularBRDF(Dggx, Gggx, fresnel, NdotV, NdotL, info.Normal);
	vec3 diffuseBRDF = float4_to_vec4(info.material.color).rgb / PI;			
	brdf_info.totalBRDF = (kd * diffuseBRDF + specularBRDF) * NdotL;

	return brdf_info;
}

//Evaluate the radiance of a pixel by directly sampling the light
vec3 DirectLight(HitInfo hit_info, in Ray ray)
{
	Light light;
	LightInfo light_info;
	int index = int(clamp(random(),0.0,0.9999f) * float(numLights));
	light = lights[index]; //randomly choose a light to sample

	vec3 Ld = vec3(0.0);
    vec3 Li = vec3(0.0);
	vec3 scatterPos = hit_info.HitPos + hit_info.Normal*0.0001;

	float r1 = random();
    float r2 = random();

    vec3 lightSurfacePos = light.position + light.u * r1 + light.v * r2;
    light_info.direction = lightSurfacePos - scatterPos;
    light_info.LightDist = length(light_info.direction);
    float distSq = light_info.LightDist * light_info.LightDist;
    light_info.direction /= light_info.LightDist;
    light_info.normal = cross(light.u,light.v);
	light_info.area = length(light_info.normal);
	light_info.normal /=  light_info.area;
	light_info.emission_color = light.emission_color;
    light_info.emission_strength = light.emission_strength;
    light_info.pdf = distSq / (light_info.area * abs(dot(light_info.normal, light_info.direction)));

	Li = light_info.emission_color * light_info.emission_strength;

	if(dot(light_info.direction,light_info.normal) < 0.0)
	{
		Ray shadowRay = Ray(scatterPos, light_info.direction);
		bool isInShadow = AnyHit(shadowRay,light_info.LightDist-0.003);
		if(!isInShadow)
		{
			float misweight = 1.0;
			BRDFInformation brdf_info = EvalBRDF(hit_info,-ray.dir,light_info.direction); //get the value of cook-torrance brdf in the light direction

			if(light_info.area > 0) //multiple importance sampling
				misweight = light_info.pdf*light_info.pdf / (light_info.pdf*light_info.pdf + brdf_info.brdfPDF*brdf_info.brdfPDF);
			if(brdf_info.brdfPDF>0.0)
				Ld += misweight * Li * brdf_info.totalBRDF/light_info.pdf;
		}
	}
	return Ld;
}
//returns color
vec3 perPixel(int numBounces, Ray ray)
{
	vec3 color=vec3(1.0);
	vec3 incommingLight = vec3(0.0);
	LightInfo light_info;
	BRDFInformation brdf_info;
	vec3 nextDirection;
	for(int k=1;k<=numBounces;k++)
	{
		HitInfo info;
		//info = ClosestHit(sphere,ray);
		info = ClosestHit(ray, light_info);
		if(info.isHit == true)
		{
			//gather the emissive material radiance
			vec3 emittedLight = float4_to_vec4(info.material.emissive_col).rgb * info.material.emissive_strength;
			incommingLight += emittedLight*color;

			//gather lights radiance
			if(info.isEmitter)
			{
				float misweight = 1.0;
				if(k>1)
					misweight = brdf_info.brdfPDF * brdf_info.brdfPDF / (brdf_info.brdfPDF * brdf_info.brdfPDF + light_info.pdf * light_info.pdf);
				incommingLight += misweight * light_info.emission_color * light_info.emission_strength * color;
				break;
			}

			if(random() > info.material.color[3]) //transparency check using the alpha channel
			{
				nextDirection = ray.dir; //do not redirect the ray
				k--;
				ray.dir = nextDirection;
				ray.origin = info.HitPos + ray.dir*0.003;
			}
			else
			{
				incommingLight += DirectLight(info, ray) * color;

				brdf_info = CalcBRDF(info, -ray.dir); //calculate and evaluate cook-torrance brdf
				if(brdf_info.brdfPDF>0.0f)
					color *= brdf_info.totalBRDF/brdf_info.brdfPDF;
				else
					break;
				nextDirection = brdf_info.reflectionDir;

				ray.dir = nextDirection;
				ray.origin = info.HitPos + info.Normal*0.003;
			}			
		}
		else{
			incommingLight += GetEnvironmentLight(ray)*color; //get the environment color
			break;
		}
	}

	return incommingLight;
}

void main()
{
	seed = uvec4(gl_GlobalInvocationID.xy, uint(frame_num + sample_count), uint(gl_GlobalInvocationID.x) + uint(gl_GlobalInvocationID.y));
	
	vec3 color = vec3(0.0); //background color
	vec2 tile_res = gl_NumWorkGroups.xy*gl_WorkGroupSize.xy;
	ivec2 uv = ivec2(gl_GlobalInvocationID.xy) ; //coordinate of of the tiles
	uv = uv + ivec2(tile_res) * ivec2(TileIndex_X,TileIndex_Y); //coordinate wrt to screen resolution (multiplied with tile-index to move to the next tile)
	vec2 coord = vec2(uv)/vec2(u_ImageWidth,u_ImageHeight); //convert to 0.0 to 1.0
	coord = coord*2.0-1.0;
	vec4 target = inverse(mat_proj) * vec4(coord,1,1); //to view space from clip-space
	
	Ray ray;
	ray.origin = camera_pos;//in world space
	ray.dir = normalize(vec3(inverse(mat_view)*vec4(target.xyz/target.w,0))); //ray dir in world space

	for(int i=0;i<numLights;i++)
	{
		lights[i] = Light(vec3(1.0,1.0,1.0), u_LightStrength, LightPos + vec3(i*6,0,0), vec3(20,0,0), vec3(0,0,20)); //fow now there is only one light
	}
	//lights[numLights-1] =  Light(vec3(1.0,1.0,1.0), u_LightStrength, LightPos + vec3(100,0,0), vec3(0,-50,0), vec3(0,0,150));
	for(int i=0; i<samplesPerPixel; i++)
		color += perPixel(num_bounces, ray); //10 bounces per ray
	color /= samplesPerPixel;

	//progressive render
	float weight = 1.0f/(sample_count+ 0.0f);
	color = color * weight + imageLoad(FinalImage,uv).rgb *(1.0f-weight);
	color = clamp(color,0.0,1.0);
	imageStore(FinalImage,uv,vec4(color,1.0));
}