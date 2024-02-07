//#shader compute
#version 460 core
layout (local_size_x = 1) in;

layout (std430, binding = 0) buffer layoutBinding0
{
	unsigned int  count;
	unsigned int  instanceCount;
	unsigned int  first;
	unsigned int  baseInstance;
}outIndirectBufferLOD0;

layout (std430, binding = 1) buffer layoutBinding1
{
	int lodCount;
}inLodCount;

uniform int VertexBufferSize;
void main()
{
	outIndirectBufferLOD0.count = VertexBufferSize;
	outIndirectBufferLOD0.instanceCount = inLodCount.lodCount;
}