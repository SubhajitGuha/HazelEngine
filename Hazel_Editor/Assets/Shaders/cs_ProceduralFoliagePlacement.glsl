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
uniform int u_instanceCount;
uniform int u_nearestDistance;
uniform int u_alignToTerrainNormal;
uniform float u_HeightMapScale;
uniform float u_zoi;
uniform float u_trunk_radius;
uniform float u_predominanceValue;
uniform float u_minScale;
uniform float u_maxScale;

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

mat4 CreateRotationMatrix(vec3 rotation)
{
	mat4 aroundX = mat4(
	1,0,0,0,
	0,cos(rotation.x),-sin(rotation.x),0,
	0,sin(rotation.x),cos(rotation.x),0,
	0,0,0,1);

	mat4 aroundY = mat4(
	cos(rotation.y),0,sin(rotation.y),0,
	0,1,0,0,
	-sin(rotation.y),0,cos(rotation.y),0,
	0,0,0,1);

	mat4 aroundZ = mat4(
	cos(rotation.z),-sin(rotation.z),0,0,
	sin(rotation.z),cos(rotation.z),0,0,
	0,0,1,0,
	0,0,0,1);

	return aroundZ * aroundY * aroundX;
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

//https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/chapter5-andersson-terrain-rendering-in-frostbite.pdf

vec3 TerrainNormal(vec2 texCoord , vec2 texelSize)
{
	float left = texture(u_HeightMap,texCoord + vec2(-texelSize.x,0.0)).r * u_HeightMapScale  *2.0 - u_HeightMapScale;
	float right = texture(u_HeightMap,texCoord + vec2(texelSize.x,0.0)).r * u_HeightMapScale *2.0 - u_HeightMapScale;
	float up = texture(u_HeightMap,texCoord + vec2(0.0 , texelSize.y)).r * u_HeightMapScale *2.0 - u_HeightMapScale;
	float down = texture(u_HeightMap,texCoord + vec2(0.0 , -texelSize.y)).r * u_HeightMapScale *2.0 - u_HeightMapScale;
	return normalize(vec3(left-right,2.0,down-up));
}
float CalculateSlope(vec3 normal)
{	
	return 1.0-normal.y; //slope is 1.0-normal.y as suggested by the dice-terrainRendering paper page 43
}

void CreateDensity(ivec2 coord)//iterates the neighbourhood pixels and assigns the density value
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

	if(m_index < u_instanceCount)
	{
		float P = 1.0; //probability of spawnning a foliage
		
		vec2 foliagePos = posBuffer.pos[m_index];
		seed = uvec4(foliagePos.x,5,foliagePos.y, 1);
		vec3 jitter_pos = vec3(foliagePos.x + randomInRange(1.0,5.0) , 0.0, foliagePos.y + randomInRange(1.0,5.0)); //jitter the foliage position for randomness

		vec2 texelSize = 1.0/textureSize(u_HeightMap,0).xy;
		vec2 uv = jitter_pos.xz * texelSize; // convert to 0-1 range (calculate the uv using the jittered position)
		float height = texture(u_HeightMap, uv).x * u_HeightMapScale; //need the height map scale
		
		vec3 terrainNormal = TerrainNormal(uv,texelSize); //get the terrain normal
		float slope = CalculateSlope(terrainNormal); //get slope from terrain normal
		slope = clamp((slope-0.5)*10.0+0.5,0.0,1.0); //add contrast to the slope value
		jitter_pos.y = height; //add the height to the jitter position

		//combine the probability
		P = P * (1.0 - slope); //dont grow on steep slopes
		P = P * u_predominanceValue;
		P = P * texture(u_DensityMap,uv).x;
		P = P * (1.0 - imageLoad(densityMap,ivec2( jitter_pos.xz)).r); //load the density map

		vec3 rotationAxis = vec3(0);
		float rotationAngle = 0;
		if(random() < P)
		{
			uint index = atomicCounterIncrement(Count_Instances); //count the total instances that are spawnning
			if(u_alignToTerrainNormal==1)
			{
				rotationAxis = -cross(vec3(0,1.0,0),terrainNormal); //get in which axis to rotate
				rotationAngle = acos(dot(vec3(0,1.0,0),terrainNormal)); //get rotation amount
			}

			inBuffer.trans[index] = CreateTranslationMatrix(jitter_pos)	* CreateRotationMatrix(rotationAngle* rotationAxis)
			* CreateRotationMatrix(vec3(0,randomInRange(-80,70),0)) * CreateScaleMatrix(randomInRange(max(u_minScale,1.0),max(u_maxScale,2.0)));
			CreateDensity(ivec2( jitter_pos.xz));
		}
	}	
}