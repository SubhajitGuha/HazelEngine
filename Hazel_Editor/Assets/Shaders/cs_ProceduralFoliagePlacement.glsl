//#shader compute
#version 460 core

layout (local_size_x = 1024) in;

layout (std430, binding = 0) buffer in_Buffer
{
	mat4 trans[];
}inBuffer;

layout (std430, binding = 1) buffer pos_Buffer
{
	vec2 pos[];
}posBuffer;

layout (binding = 2) uniform atomic_uint Count_Instances;

layout (binding = 3, r16f) uniform image2D densityMap;

uniform sampler2D u_DensityMap;
uniform sampler2D u_HeightMap;
uniform float u_HeightMapScale;
uniform vec3 u_PlayerPos;
uniform int u_instanceCount;
uniform int u_nearestDistance;
uniform float u_zoi;
uniform float u_trunk_radius;
uniform int offset;

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
float randomInRange(float _min,float _max)
{
	return random()*(_max -_min)+_min;
}
//column major format
mat4 CreateTranslationMatrix(vec3 pos)
{
	return mat4(1,0,0,0,
				0,1,0,0,
				0,0,1,0,
				pos.x,pos.y,pos.z,1);
}

mat4 CreateRotationMatrix(float x, float y, float z)
{
	x= radians(x);
	y= radians(y);
	z= radians(z);

	mat4 aroundX = mat4(
	1,0,0,0,
	0,cos(x),sin(x),0,
	0,-sin(x),cos(x),0,
	0,0,0,1);

	mat4 aroundY = mat4(
	cos(y),0,-sin(y),0,
	0,1,0,0,
	sin(y),0,cos(y),0,
	0,0,0,1);

	mat4 aroundZ = mat4(
	cos(z),sin(z),0,0,
	-sin(z),cos(z),0,0,
	0,0,1,0,
	0,0,0,1);

	return aroundX * aroundY * aroundZ;
}

mat4 CreateScaleMatrix(float scale)
{
	return mat4(
	scale,0,0,0,
	0,scale,0,0,
	0,0,scale,0,
	0,0,0,1
	);
}

const int dist = u_nearestDistance;
const float trunk_radius = u_trunk_radius;
const float zoi = u_zoi;

void CreateDensity(ivec2 coord)
{
	for(int i=-dist; i<=dist;i++)
	{
		for(int j=-dist; j<=dist; j++)
		{
			float distanceField = 1.0 - clamp( (distance(coord,coord + ivec2(i,j)) - trunk_radius) / (zoi-trunk_radius),0.0,1.0);
			vec3 color = imageLoad(densityMap, coord + ivec2(i,j)).rgb;
			vec3 combined = clamp(vec3(distanceField) + color,0.0,1.0);
			imageStore(densityMap,coord + ivec2(i,j), vec4(combined,1.0));
		}
	}
}

void main()
{
	int m_index = int(gl_GlobalInvocationID.x);
	uint prev_count = 0;
	//uint initilize = 0;
	if(m_index == 0)
	{
		prev_count = atomicCounter(Count_Instances);
	}
	//m_index += u_instanceCountOffset;

	if(m_index < u_instanceCount)
	{
		float P = 1.0; //probability of spawnning a foliage
		
		vec2 foliagePos = posBuffer.pos[m_index];
		//foliagePos = max(foliagePos + u_PlayerPos.xz - vec2(gl_NumWorkGroups.x*32/2), foliagePos); //offset by player position
		vec2 uv = foliagePos / textureSize(u_HeightMap,0).xy; // convert to 0-1 range
		float height = texture(u_HeightMap, uv).x * u_HeightMapScale; //need the height map scale
		seed = uvec4(foliagePos.x,height,foliagePos.y, 1);
		vec3 jitter_pos = vec3(foliagePos.x + randomInRange(1.0,5.0) , height, foliagePos.y + randomInRange(1.0,5.0));

		P = P * (1.0 - imageLoad(densityMap,ivec2( jitter_pos.xz)).r); //load the density map
		if(random() < P)
		{
			uint index = atomicCounterIncrement(Count_Instances); //count the total instances that are spawnning
			inBuffer.trans[index] = CreateTranslationMatrix(jitter_pos) 
			* CreateRotationMatrix(0,randomInRange(5.0,180.0),0.0) * CreateScaleMatrix(randomInRange(1.0,2.0));
			CreateDensity(ivec2( jitter_pos.xz));
		}
	}	
}