//#shader compute
#version 460 core
layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) readonly buffer in_Buffer
{
	mat4 trans[];
}inBuffer;

layout (std430, binding = 1) readonly buffer vote_Buffer
{
	int vote[];
}voteBuffer;

layout (std430, binding = 2) readonly buffer scan_Buffer
{
	int scan[];
}scanBuffer;

layout (std430, binding = 3) writeonly buffer out_Buffer
{
	mat4 out_trans[];
}outBuffer;

uniform int offset;

void main()
{
	int index = int(gl_GlobalInvocationID.x) + offset;

	//outBuffer.out_trans[outIndex] = mat4(0.0);		

	if(voteBuffer.vote[index] == 1)
	{
		int outIndex = scanBuffer.scan[index]; //scan array contains the indices that we get after prefix sum
		outBuffer.out_trans[outIndex] = inBuffer.trans[index];
	}
	
}