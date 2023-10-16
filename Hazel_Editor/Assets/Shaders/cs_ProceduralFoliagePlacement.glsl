#shader compute
#version 430 core

layout (local_size_x = 32, local_size_y = 32 ,local_size_z = 1) in;

layout (std430, binding = 0) buffer in_Buffer
{
	mat4 trans[];
}inBuffer;

layout (std430, binding = 1) buffer pos_Buffer
{
	vec2 pos[];
}posBuffer;

uniform sampler2D u_DensityMap;
uniform sampler2D u_HeightMap;
uniform float u_HeightMapScale;
uniform vec3 u_PlayerPos;
uniform float u_instanceCount;
//uniform float u_radius;
uniform int offset;

uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

// Pseudo-random value in half-open range [0:1].
float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float randomInRange (float min, float max, float seed)
{
	return random(seed) * (max-min) + min;
}

//column major format
mat4 CreateTranslationMatrix(float x, float y, float z)
{
	return mat4(1,0,0,0,
				0,1,0,0,
				0,0,1,0,
				x,y,z,1);
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

void main()
{
	int m_index = int(gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x + gl_GlobalInvocationID.x);

	//int density = int(texture(u_DensityMap,uv).x * 10);
	//if(density <= 3)
	//{
	//	inBuffer.trans[m_index] = mat4(1.0,0,0,0, 0,1.0,0,0, 0,0,1.0,0, -1000,-1000,-1000,1);
	//	return;
	//}

	//barrier();

	if(m_index < u_instanceCount)
	{
		vec2 foliagePos = posBuffer.pos[m_index];
		foliagePos = max(foliagePos + u_PlayerPos.xz - vec2(gl_NumWorkGroups.x*32/2), foliagePos); //offset by player position
		vec2 uv = foliagePos / textureSize(u_HeightMap,0).x; // convert to 0-1 range
		float height = texture(u_HeightMap, uv).x * u_HeightMapScale; //need the height map scale

		inBuffer.trans[m_index] = CreateTranslationMatrix(foliagePos.x + randomInRange(1.0,5.0,m_index) , height, foliagePos.y + randomInRange(1.0,5.0,m_index + 1)) 
		* CreateRotationMatrix(randomInRange(5.0,15.0,m_index + 4),randomInRange(5.0,45.0,m_index + 3),0.0);//* CreateScaleMatrix(randomInRange(1.0,2.0,m_index + 4));
	}
	
}