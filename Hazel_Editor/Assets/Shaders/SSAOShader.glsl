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
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D noisetex;
uniform mat4 u_projection;
uniform mat4 u_ProjectionView;
uniform mat4 u_View;
uniform vec3 u_CamPos;

//For foliages only
uniform sampler2DArray alpha_texture; //for foliages
uniform int isFoliage;//For foliages

float radius = 0.7;
float bias = 0.085;//tested value

const float Threshold_dist = 1000.0;
void main()
{
	vec4 m_pos = vec4(texture(gPosition,tcord).xyz , 1.0);
	
	//if(abs(length(u_CamPos - m_pos.xyz))>Threshold_dist) // SSAO rendering threshold beyond this the ambiant color will be pure white 
	//{
	//	color =  vec4(1.0);
	//	return;
	//}
	
	//// For foliage ^_^
	//int index = int (m_materialindex);
	//vec3 alpha = texture(alpha_texture , vec3(tcord,index)).xyz;
	//if(isFoliage == 1 && alpha.r <= 0.06)
	//	discard;

	vec4 position = m_pos; // sample the position map

	vec2 noiseScale = vec2(ScreenWidth/4.0 , ScreenHeight/4.0);
	float occlusion = 0.0;
	vec3 FragPos = position.xyz;
	vec3 RandomVec = texture(noisetex , tcord*noiseScale).xyz;
	vec3 normal = normalize(texture(gNormal,tcord).xyz);

	vec3 tangent = normalize(RandomVec - normal * dot(RandomVec , normal));
	vec3 bitangent = cross(normal , tangent);
	mat3 TBN = mat3(tangent , bitangent, normal);
	for(int i=0; i<RANDOM_SAMPLES_SIZE; i++)
	{
		 //view space
		vec4 SamplePoint = vec4(FragPos + TBN * Samples[i] * vec3(radius),1.0);

		vec4 offset = u_projection * SamplePoint;
		offset.xyz = offset.xyz/offset.w;
		offset.xyz = offset.xyz*0.5 + vec3(0.5); // 0 - 1 range

		vec3 depth = texture(gPosition,offset.xy).rgb;

		float RangeCheck = smoothstep(0.0,1.0 , radius/abs(FragPos.z - depth.z)); //calc occlusion if depth val is within the radius
			occlusion += (depth.z >= SamplePoint.z + bias ? 1.0:0.0) * RangeCheck;
	}
	occlusion = 1.0 - occlusion/RANDOM_SAMPLES_SIZE;
	vec3 output = vec3(pow(occlusion,4.0));

	color =  vec4(output,1.0);	
}