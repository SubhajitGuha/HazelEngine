//#shader compute
#version 460 core

layout (local_size_x = 64) in;

layout (std430, binding = 0) buffer in_transBuffer
{
	mat4 inTrans[];
}inTransBuffer;

layout (std430, binding = 1) buffer out_transBufferLOD0
{
	mat4 outTransLOD0[];
}outTransBufferLOD0;

layout (std430, binding = 2) buffer out_transBufferLOD1
{
	mat4 outTransLOD1[];
}outTransBufferLOD1;

layout (binding = 3) uniform atomic_uint Counter_Lod0;
layout (binding = 4) uniform atomic_uint Counter_Lod1;

layout (std430, binding = 5) buffer layoutBinding5
{
	int totalPrefixSum; //total instances to be rendered
}inTotalPrefixSum;

uniform vec3 u_camPos;
uniform mat4 u_ModelMat; //this is the terrain model matrix (if terrain position or rotation changes then lods will be affected too
uniform int u_VertexBufferLength0;
uniform int u_VertexBufferLength1;
uniform float u_LOD0Distance;

void main()
{
	int index = int(gl_GlobalInvocationID.x);
	if(index > inTotalPrefixSum.totalPrefixSum)				
		return;
	
	uint initilize = 0;
	if(index == 0)
	{
		atomicCounterExchange(Counter_Lod0, initilize);
		atomicCounterExchange(Counter_Lod1, initilize);
	}

	mat4 matrix = u_ModelMat * inTransBuffer.inTrans[index]; //in world-space
	vec3 pos = (matrix * vec4(0,0,0,1)).xyz;//vec3(matrix[3][0], matrix[3][1], matrix[3][2]);
	float dist = distance(pos,u_camPos);

	if(dist<u_LOD0Distance) //only 2 LODs for now
	{
		uint counter = atomicCounterIncrement(Counter_Lod0);
		outTransBufferLOD0.outTransLOD0[counter] = inTransBuffer.inTrans[index];
	}
	else
	{
		uint counter = atomicCounterIncrement(Counter_Lod1);		
		outTransBufferLOD1.outTransLOD1[counter] = inTransBuffer.inTrans[index];
	}
}