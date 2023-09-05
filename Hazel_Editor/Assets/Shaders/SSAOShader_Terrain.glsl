#shader vertex
#version 410 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 cord;

flat out float m_materialindex;

out vec2 TexCoord;

void main()
{
	gl_Position = vec4(pos,1.0);
	TexCoord = cord;
}

#shader tessellation control
#version 410 core
layout (vertices = 4) out;
const float MAX_TESS_LEVEL = 64;
const float MIN_TESS_LEVEL = 4;
const float MAX_CAM_DIST = 1000;
const float MIN_CAM_DIST = 0;

uniform mat4 u_Model;
uniform mat4 u_View;

in vec2 TexCoord[];
out TCS_Data
{
	vec2 TexCoord_TCS;
} tcs_data[];


void main()
{
	if(gl_InvocationID == 0)
	{

		vec4 p1 = u_View * u_Model * gl_in[0].gl_Position;
		vec4 p2 = u_View * u_Model * gl_in[1].gl_Position;
		vec4 p3 = u_View * u_Model * gl_in[2].gl_Position;
		vec4 p4 = u_View * u_Model * gl_in[3].gl_Position;
		
		float dist01 = clamp( (abs(p1.z) - MIN_CAM_DIST) / (MAX_CAM_DIST-MIN_CAM_DIST), 0.0, 1.0);
		float dist02 = clamp( (abs(p2.z) - MIN_CAM_DIST) / (MAX_CAM_DIST-MIN_CAM_DIST), 0.0, 1.0);
		float dist03 = clamp( (abs(p3.z) - MIN_CAM_DIST) / (MAX_CAM_DIST-MIN_CAM_DIST), 0.0, 1.0);
		float dist04 = clamp( (abs(p4.z) - MIN_CAM_DIST) / (MAX_CAM_DIST-MIN_CAM_DIST), 0.0, 1.0);

		float TessValue01 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist01,dist03));
		float TessValue02 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist01,dist02));
		float TessValue03 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist02,dist04));
		float TessValue04 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist04,dist03));

		gl_TessLevelOuter[0] = TessValue01;
		gl_TessLevelOuter[1] = TessValue02;
		gl_TessLevelOuter[2] = TessValue03;
		gl_TessLevelOuter[3] = TessValue04;
							   
		gl_TessLevelInner[0] = max(TessValue02,TessValue04);
		gl_TessLevelInner[1] = max(TessValue01,TessValue03);
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	tcs_data[gl_InvocationID].TexCoord_TCS = TexCoord[gl_InvocationID];
}


#shader tessellation evaluation
#version 410 core
layout (quads,fractional_even_spacing ,ccw) in;

in TCS_Data
{
	vec2 TexCoord_TCS;
} tcs_data[];

out Frag_Data
{
	vec2 texCoord;
	vec4 pos;
}frag_data;

uniform mat4 u_ProjectionView;
uniform sampler2D u_HeightMap;
uniform mat4 u_Model;
uniform float HEIGHT_SCALE;

vec2 texture_size;

vec4 Interpolate(vec4 v0, vec4 v1, vec4 v2, vec4 v3)
{
	vec4 a = mix(v0,v1,gl_TessCoord.x);
	vec4 b = mix(v2,v3,gl_TessCoord.x);
	return mix(a,b,gl_TessCoord.y);
}

vec2 Interpolate(vec2 v0, vec2 v1, vec2 v2, vec2 v3)
{
	vec2 a = mix(v0,v1,gl_TessCoord.x);
	vec2 b = mix(v2,v3,gl_TessCoord.x);
	return mix(a,b,gl_TessCoord.y);
}

void main()
{
	//in Light space
	vec4 oldPos =  u_Model * Interpolate(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position, gl_in[3].gl_Position);
	vec2 texCoord = Interpolate(tcs_data[0].TexCoord_TCS, tcs_data[1].TexCoord_TCS, tcs_data[2].TexCoord_TCS, tcs_data[3].TexCoord_TCS);
	float Height = texture(u_HeightMap,texCoord).r * HEIGHT_SCALE;
	vec4 newPos = u_Model * vec4(0,Height,0,0); //as proj_view and model matrix is same for all vertex
	
	frag_data.pos = oldPos + newPos;
	frag_data.texCoord = texCoord;
	gl_Position = u_ProjectionView * (oldPos + newPos);
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

in vec3 m_Normal;
in Frag_Data
{
	vec2 texCoord;
	vec4 pos;
}frag_data;

#define RANDOM_SAMPLES_SIZE 64

uniform float ScreenWidth;
uniform float ScreenHeight;

uniform vec3 Samples[RANDOM_SAMPLES_SIZE];
uniform sampler2D GPosition;
uniform sampler2D noisetex;
uniform mat4 u_projection;
uniform mat4 u_ProjectionView;
uniform vec3 u_CamPos;
uniform sampler2D u_HeightMap;
uniform float HEIGHT_SCALE;

float radius = 0.7;
float bias = 0.85;//tested value

const float Threshold_dist = 1000.0;

vec3 CalculateNormal(vec2 texCoord , vec2 texelSize)
{
	float left = texture(u_HeightMap,texCoord + vec2(-texelSize.x,0.0)).r * HEIGHT_SCALE  *2.0 - HEIGHT_SCALE;
	float right = texture(u_HeightMap,texCoord + vec2(texelSize.x,0.0)).r * HEIGHT_SCALE *2.0 - HEIGHT_SCALE;
	float up = texture(u_HeightMap,texCoord + vec2(0.0 , texelSize.y)).r * HEIGHT_SCALE *2.0 - HEIGHT_SCALE;
	float down = texture(u_HeightMap,texCoord + vec2(0.0 , -texelSize.y)).r * HEIGHT_SCALE *2.0 - HEIGHT_SCALE;

	return normalize(vec3(down-up,2.0,left-right));
}

void main()
{
	if(abs(length(u_CamPos - frag_data.pos.xyz))>Threshold_dist) // SSAO rendering threshold beyond this the ambiant color will be pure white 
	{
		color =  vec4(1.0);
		return;
	}
	

	vec4 coordinate = u_ProjectionView * frag_data.pos;
	coordinate.xyz /= coordinate.w;
	coordinate.xyz = coordinate.xyz*0.5 + 0.5;

	vec4 position = texture(GPosition,coordinate.xy);// sample the position map

	vec2 noiseScale = vec2(ScreenWidth/4.0 , ScreenHeight/4.0);
	float occlusion = 0.0;
	vec3 FragPos = position.xyz;
	vec3 RandomVec = texture(noisetex , coordinate.xy*noiseScale).xyz;
	vec2 texture_size = textureSize(u_HeightMap,0);	//get texture dimension
	vec3 normal = CalculateNormal(frag_data.texCoord , vec2(1/texture_size.x));

	vec3 tangent = normalize(RandomVec - normal*dot(RandomVec , normal));
	vec3 bitangent = cross(normal , tangent);
	mat3 TBN = mat3(tangent , bitangent, normal);
	for(int i=0; i<RANDOM_SAMPLES_SIZE; i++)
	{
		 //view space
		vec4 SamplePoint = vec4(FragPos + TBN * Samples[i] * vec3(radius),1.0);

		vec4 offset = u_projection * SamplePoint;
		offset.xyz = offset.xyz/offset.w;
		offset.xyz = offset.xyz*0.5 + vec3(0.5); // 0 - 1 range

		vec3 depth = texture(GPosition,offset.xy).rgb;

		float RangeCheck = smoothstep(0.0,1.0 , radius/abs(FragPos.z - depth.z)); //calc occlusion if depth val is within the radius
			occlusion += (depth.z >= SamplePoint.z + bias ? 1.0:0.0) * RangeCheck;
	}
	occlusion = 1.0 - occlusion/RANDOM_SAMPLES_SIZE;
	vec3 output = vec3(pow(occlusion,4.0));

	//output = vec3(1.0) - exp(-output * 2);//exposure
	//output = pow(output, vec3(1.0/2.2)); //Gamma correction
	color =  vec4(output,1.0);
	
}