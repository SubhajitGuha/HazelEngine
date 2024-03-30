//#shader compute
#version 460 core

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) readonly buffer layoutBinding0
{
	mat4 trans[];
}inBuffer;

layout (std430, binding = 1) buffer layoutBinding1
{
	uint instanceCount;
}totoalInstanceCount;

layout (std430, binding = 2) writeonly buffer layoutBinding2
{
	mat4 out_trans[];
}outBuffer;

layout (binding = 3) uniform atomic_uint totalInstancesInsideFrustum;

uniform vec3 camPos;
uniform mat4 u_ViewProjection;
uniform float u_cullDistance;
uniform vec3 aabb_min;
uniform vec3 aabb_max;

bool within(float left, float point, float right)
{
	bool ieq1 = (point >= left);
	bool ieq2 = (point <= right);
	
	return ieq1 && ieq2;
}

void main()
{
	int index = int(gl_GlobalInvocationID.x);

	uint initilize = 0;	
	if(index==0)
		atomicCounterExchange(totalInstancesInsideFrustum,initilize);

	if(index>totoalInstanceCount.instanceCount)
		return;

	bool inview = false;
	mat4 instance_matrix = inBuffer.trans[index];
	vec3 foliage_pos = vec3(instance_matrix[3][0],instance_matrix[3][1],instance_matrix[3][2]);
	
	vec4 corners[8] = {
        vec4(aabb_min.x, aabb_min.y, aabb_min.z, 1.0), // x y z
        vec4(aabb_max.x, aabb_min.y, aabb_min.z, 1.0), // X y z
        vec4(aabb_min.x, aabb_max.y, aabb_min.z, 1.0), // x Y z
        vec4(aabb_max.x, aabb_max.y, aabb_min.z, 1.0), // X Y z
												
        vec4(aabb_min.x, aabb_min.y, aabb_max.z, 1.0), // x y Z
        vec4(aabb_max.x, aabb_min.y, aabb_max.z, 1.0), // X y Z
        vec4(aabb_min.x, aabb_max.y, aabb_max.z, 1.0), // x Y Z
        vec4(aabb_max.x, aabb_max.y, aabb_max.z, 1.0), // X Y Z
    };

	for(int i=0;i<8;i++)
	{
		vec4 clipSpace = u_ViewProjection * instance_matrix * corners[i];		
		inview = inview || (within(-clipSpace.w,clipSpace.x,clipSpace.w) && within(-clipSpace.w,clipSpace.y,clipSpace.w) && within(-clipSpace.w,clipSpace.z,clipSpace.w));
	}
	//int inview = clipSpace.x < -0.2f || clipSpace.x > 1.2f || clipSpace.z <= -0.1f ? 0 : 1;

	if(inview && distance(foliage_pos,camPos) <= u_cullDistance)
	{
		uint count = atomicCounterIncrement(totalInstancesInsideFrustum);
		outBuffer.out_trans[count] = instance_matrix;
	}
		
}