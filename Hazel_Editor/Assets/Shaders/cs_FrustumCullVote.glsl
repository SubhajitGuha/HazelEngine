#shader compute
#version 430 core
layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) readonly buffer in_Buffer
{
	mat4 trans[];
}inBuffer;

layout (std430, binding = 1) writeonly buffer vote_Buffer
{
	int vote[];
}voteBuffer;

uniform vec3 camPos;
uniform mat4 u_ViewProjection;
uniform int offset; //variable to shift the array index by (1024) as we do multiple dispatches of size 1024

void main()
{
	int index = int(gl_GlobalInvocationID.x) + offset;	

	mat4 instance_matrix = inBuffer.trans[index];
	vec3 foliage_pos = vec3(instance_matrix[3][0],instance_matrix[3][1],instance_matrix[3][2]);

	vec4 clipSpace = u_ViewProjection * vec4(foliage_pos,1.0);
	clipSpace.xyz /= clipSpace.w;
	clipSpace.xy = clipSpace.xy * 0.5 + vec2(0.5);
	clipSpace.z = clipSpace.w;

	int inview = clipSpace.x < -0.2f || clipSpace.x > 1.2f || clipSpace.z <= -0.1f ? 0 : 1;
	voteBuffer.vote[index] = 0;		

	//if(distance(foliage_pos,camPos) <= 200.0)
	if(inview == 1 )
	{
		voteBuffer.vote[index] = 1;		
	}
		
}