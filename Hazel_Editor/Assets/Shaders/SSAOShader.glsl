#shader vertex
#version 410 core
layout (location = 0) in vec4 position;
layout (location = 1) in vec4 cord;

out vec2 tcord;

void main()
{
	gl_Position = position;
	tcord = cord.xy;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

in vec2 tcord;

#define RANDOM_SAMPLES_SIZE 64

uniform float ScreenWidth;
uniform float ScreenHeight;

uniform vec3 Samples[RANDOM_SAMPLES_SIZE];
uniform sampler2D depthBuffer;
uniform sampler2D gNormal;
uniform sampler2D noisetex;
uniform mat4 u_projection;
uniform vec3 u_CamPos;

//For foliages only
uniform sampler2DArray alpha_texture; //for foliages
uniform int isFoliage;//For foliages

float radius = 0.4;
float bias = 0.085;//tested value

const float Threshold_dist = 1000.0;

vec4 GetViewSpacePosition(vec2 texture_coord)
{
	float z = texture(depthBuffer,texture_coord).r;
	vec4 clip_space = vec4(texture_coord*2.0-1.0,z*2.0-1.0,1.0);
	vec4 view_space = inverse(u_projection) * clip_space;
	view_space /= view_space.w;
	return view_space;
}
void main()
{
	vec2 noiseScale = vec2(ScreenWidth/4.0 , ScreenHeight/4.0);
	float occlusion = 0.0;
	vec3 FragPos = GetViewSpacePosition(tcord).xyz;
	vec3 RandomVec = texture(noisetex , tcord*noiseScale).xyz;
	vec3 normal = normalize(texture(gNormal,tcord).xyz);

	vec3 tangent = normalize(RandomVec - normal * dot(RandomVec , normal));
	//vec3 up = abs(normal.x) > 0.99 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    //vec3 tangent = normalize(cross(up, normal));
	vec3 bitangent = cross(normal , tangent);
	mat3 TBN = mat3(tangent , bitangent, normal);
	for(int i=0; i<RANDOM_SAMPLES_SIZE; i++)
	{
		 //view space
		vec4 SamplePoint = vec4(FragPos + TBN * Samples[i] * vec3(radius),1.0);

		vec4 offset = u_projection * SamplePoint;
		offset.xyz = offset.xyz/offset.w;
		offset.xyz = offset.xyz*0.5 + vec3(0.5); // 0 - 1 range

		vec3 depth = GetViewSpacePosition(offset.xy).xyz; //sample position from the offset coord.

		float RangeCheck = smoothstep(0.0,1.0 , radius/abs(FragPos.z - depth.z)); //calc occlusion if depth val is within the radius
			occlusion += (depth.z >= SamplePoint.z + bias ? 1.0:0.0) * RangeCheck;
	}
	occlusion = 1.0 - occlusion/RANDOM_SAMPLES_SIZE;
	vec3 output = vec3(pow(occlusion,2.0));

	color =  vec4(output,1.0);	
}