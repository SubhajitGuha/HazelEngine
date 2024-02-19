//#shader compute
#version 460 core

layout (local_size_x=32,local_size_y=32) in;

layout (binding = 0, r16f) writeonly uniform image2D densityMap;

void main()
{
	ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
	imageStore(densityMap,coord,vec4(0.0,0.0,0.0,1.0));
}