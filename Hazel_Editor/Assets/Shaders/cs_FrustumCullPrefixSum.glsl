//#shader compute
#version 460 core

layout (local_size_x = 512, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) readonly buffer in_Buffer
{
	int in_arr[];
}inBuffer;

layout (std430, binding = 1) writeonly buffer sum_Buffer
{
	int sum[];
}sumBuffer;

layout (std430, binding = 2) buffer totalSum_Buffer //accmulated sum that needs to be added to next dispatch of this shader
{
	int TotSum;
}totalSum;

uniform int stride; //the offset amount must be offset for (vote/compact) /2 (i.e if offset for other shaders is 1024 then here it will be 1024/2 = 512)
shared int tmp[1024]; 

void main()
{
	//parallel algorithm taken from
	//https://users.umiacs.umd.edu/~ramani/cmsc828e_gpusci/ScanTalk.pdf
	if(stride == 0)
		totalSum.TotSum = 0;
	int index = int(gl_GlobalInvocationID.x);

	//copy the values from the input (vote buffer)
	tmp[2*index] = inBuffer.in_arr[2*(index + stride)];
	tmp[2*index + 1] = inBuffer.in_arr[2*(index + stride) +1];

	int num_elements = 1024; //2*total threads (as tot threads = size/2) hardcodded
	int offset = 1;
	
	//sum up along the tree; parallel reduction
	for(int d = (num_elements >> 1); d>0; d >>= 1 ) // d >>= means right shift and assign to d
	{
		barrier();
		//memoryBarrierShared();
	
		if(index < d)
		{
			int ai = offset * (2 * index + 1) - 1;
            int bi = offset * (2 * index + 2) - 1;
			tmp[bi] += tmp[ai];
		}
		offset <<= 1;
	}
	
	if(index == 0)
	{
		tmp[num_elements-1]=0;
	}
	
	//deduce down along the tree
	for (int d = 1; d < num_elements; d *= 2) {
        offset >>= 1;
		barrier();
		//memoryBarrierShared();
	
        if (index < d) {
            int ai = offset * (2 * index + 1) - 1;
            int bi = offset * (2 * index + 2) - 1;
            int t = tmp[ai];
            tmp[ai] = tmp[bi];
            tmp[bi] += t;
        }
    }
	barrier();
	//memoryBarrierShared();
	
	sumBuffer.sum[2*(index + stride)] = tmp[2 * index] + totalSum.TotSum;
    sumBuffer.sum[2*(index + stride) + 1] = tmp[2 * index + 1] + totalSum.TotSum;

	if(index == gl_WorkGroupSize.x-1)
		totalSum.TotSum += tmp[num_elements-1]; //add the sum with the last prefixSum
}