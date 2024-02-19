//#shader compute
#version 460 core

layout (local_size_x = 32, local_size_y = 32 ,local_size_z = 1) in;

layout (std430, binding = 0) buffer in_Buffer
{
	mat4 trans[];
}inBuffer;

layout (binding = 1) uniform atomic_uint counter_instances;

uniform sampler2D u_DensityMap;
uniform sampler2D u_HeightMap;
uniform sampler2D u_BlueNoise;
uniform float u_HeightMapScale;
uniform vec3 u_PlayerPos;
uniform float u_instanceCount;
//uniform float u_radius;
uniform float u_spacing;

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

vec3 CalculateNormal(vec2 texCoord , vec2 texelSize)
{
	float left = texture(u_HeightMap,texCoord + vec2(-texelSize.x,0.0)).r * u_HeightMapScale  *2.0 - u_HeightMapScale;
	float right = texture(u_HeightMap,texCoord + vec2(texelSize.x,0.0)).r * u_HeightMapScale *2.0 - u_HeightMapScale;
	float up = texture(u_HeightMap,texCoord + vec2(0.0 , texelSize.y)).r * u_HeightMapScale *2.0 - u_HeightMapScale;
	float down = texture(u_HeightMap,texCoord + vec2(0.0 , -texelSize.y)).r * u_HeightMapScale *2.0 - u_HeightMapScale;

	return normalize(vec3(down-up,2.0,left-right));
}

vec3 CalculateTangent(vec2 texCoord , vec2 texelSize)
{
	float left = texture(u_HeightMap,texCoord + vec2(-texelSize.x,0.0)).r * u_HeightMapScale  *2.0 - u_HeightMapScale;
	float right = texture(u_HeightMap,texCoord + vec2(texelSize.x,0.0)).r * u_HeightMapScale *2.0 - u_HeightMapScale;
	float up = texture(u_HeightMap,texCoord + vec2(0.0 , texelSize.y)).r * u_HeightMapScale *2.0 - u_HeightMapScale;
	float down = texture(u_HeightMap,texCoord + vec2(0.0 , -texelSize.y)).r * u_HeightMapScale *2.0 - u_HeightMapScale;

	return normalize(vec3(2.0,0.0,left-right));
}

mat4 TBN(vec2 texCoord , vec2 texelSize)
{
	vec3 N = CalculateNormal(texCoord,texelSize);	
	vec3 T = CalculateTangent(texCoord, texelSize);
	vec3 B = cross(N,T);

	return mat4(vec4(T,0),vec4(B,0),vec4(N,0),vec4(0,0,0,1));
}

float GetSteepness(vec2 texCoord , vec2 texelSize)
{
	float height = texture(u_HeightMap,texCoord).x;
	float dx = texture(u_HeightMap, texCoord + vec2(texelSize.x,0.0)).x - height;
	float dy = texture(u_HeightMap, texCoord + vec2(0.0,texelSize.y)).x - height;

	return abs(dy/dx);
}

void main()
{
	int m_index = int(gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x + gl_GlobalInvocationID.x);

	uint initilize = 0;
	if(m_index == 0)
	{
		atomicCounterExchange(counter_instances, initilize);
	}

	
	vec2 foliagePos = vec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y) * u_spacing; //2.0 is the spacing that I need for certain foliage
	foliagePos = max(foliagePos + u_PlayerPos.xz - vec2(gl_NumWorkGroups.x*32/2), foliagePos); //offset by player position
	vec2 texelSize = 1.0/textureSize(u_HeightMap,0);
	vec2 uv = foliagePos * texelSize; // convert to 0-1 range
	float height = texture(u_HeightMap, uv).x * u_HeightMapScale; //need the height map scale
	seed = uvec4(foliagePos.x,height,foliagePos.y,m_index);
	vec3 pos = vec3(foliagePos.x + randomInRange(1.0,5.0) , height, foliagePos.y + randomInRange(1.0,5.0));
	if((1.0 - CalculateNormal(uv,texelSize).y)> 0.4)
		pos = pos + CalculateNormal(uv,texelSize)*2; 
	if(random() < texture(u_DensityMap,pos.xz/2048.0).x)
	{

		uint index = atomicCounterIncrement(counter_instances);
		inBuffer.trans[m_index] = CreateTranslationMatrix(pos)
		* CreateRotationMatrix(randomInRange(0.0,5.0),randomInRange(5.0,10.0),randomInRange(0.0,5.0)) * CreateScaleMatrix(randomInRange(1.0,2.0));		
	}
}