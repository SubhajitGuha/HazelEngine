#shader compute
#version 460 core

layout (local_size_x = 1024) in;

layout (std430, binding = 0) buffer vote_Buffer
{
	int vote[];
}voteBuffer;

layout (std430, binding = 1) buffer sum_Buffer
{
	int sum[];
}sumBuffer;

layout (std430, binding = 2) buffer out_Buffer
{
	mat4 trans[];
}outBuffer;

void main()
{
	int index = int(gl_GlobalInvocationID.x);

	//voteBuffer.vote[index] = 0;
	//sumBuffer.sum[index] = 0;
	outBuffer.trans[index][0][0] = 0;
	outBuffer.trans[index][1][1] = 0;
	outBuffer.trans[index][2][2] = 0;
	outBuffer.trans[index][3][3] = 0;

}