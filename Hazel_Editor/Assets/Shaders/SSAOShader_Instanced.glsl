#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 cord;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 Tangent;
layout (location = 4) in vec3 BiTangent;
layout (location = 5) in float materialindex;
layout (location = 6) in mat4 instance_mm;

out vec2 tcord;
out vec3 m_Normal;
out vec4 m_pos;
flat out float m_materialindex;

uniform mat4 u_ProjectionView;
uniform mat4 u_projection;
uniform mat4 u_Model;
uniform mat4 u_View;
uniform sampler2D Noise;
uniform float u_Time;
float amplitude=40;
float wsAmplitude=0.6;

mat4 CreateScaleMatrix(float scale)
{
	return mat4(
	scale,0,0,0,
	0,scale,0,0,
	0,0,scale,0,
	0,0,0,1
	);
}

mat4 CreateRotationMat(float x, float y, float z)
{
	x= radians(x);
	y= radians(y);
	z= radians(z);

	mat4 aroundX = mat4(
	1,0,0,0,
	0,cos(x),sin(x),0,
	0,-sin(x),cos(x),0,
	0,0,0,1);

	mat4 aroundY = mat4(
	cos(y),0,-sin(y),0,
	0,1,0,0,
	sin(y),0,cos(y),0,
	0,0,0,1);

	mat4 aroundZ = mat4(
	cos(z),sin(z),0,0,
	-sin(z),cos(z),0,0,
	0,0,1,0,
	0,0,0,1);

	return aroundX * aroundY * aroundZ;
}

void main()
{
	mat4 wsGrass = u_Model * instance_mm;
	vec4 wsVertexPos = wsGrass * pos;
	vec3 origin = vec3(wsGrass[3][0],wsGrass[3][1],wsGrass[3][2]);
	float factor = distance(wsVertexPos.xyz , origin)/10;	
	if(factor<0)
		factor = 0;

	vec3 coord = mod(abs(wsVertexPos.xyz),256);
	coord/=vec3(256);

	vec3 noise = texture(Noise , coord.xz*2 ).rgb*2;
	vec3 rotVal = vec3(sin(u_Time + noise.r)*0.5+0.5+1) *factor*amplitude;
	vec3 ws_rotVal = vec3(noise.r*sin(u_Time)*0.5+0.5) *factor*wsAmplitude;

	mat4 ws_rot = CreateRotationMat(0, 0, ws_rotVal.y);
	mat4 rot = CreateRotationMat(0, rotVal.y, 0);

	float val = texture(Noise,coord.xz*10).r;

	gl_Position = u_ProjectionView * ws_rot * wsGrass * rot * CreateScaleMatrix(val*2) * pos;

	m_pos = ws_rot * wsGrass * rot * CreateScaleMatrix(val*2) * pos;
	tcord = cord;
	m_Normal = (normalize(u_View * wsGrass * vec4(Normal,0.0))).xyz;
	m_materialindex = materialindex;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

in vec3 m_Normal;
in vec2 tcord;
in vec4 m_pos;
flat in float m_materialindex;

#define RANDOM_SAMPLES_SIZE 64

uniform float ScreenWidth;
uniform float ScreenHeight;

uniform vec3 Samples[RANDOM_SAMPLES_SIZE];
uniform sampler2D GPosition;
uniform sampler2D noisetex;
uniform mat4 u_projection;
uniform mat4 u_ProjectionView;
uniform vec3 u_CamPos;

//For foliages only
uniform sampler2DArray alpha_texture; //for foliages

float radius = 1.7;
float bias = 0.085;//tested value

const float Threshold_dist = 1000.0;
void main()
{
	if(abs(length(u_CamPos - m_pos.xyz))>Threshold_dist) // SSAO rendering threshold beyond this the ambiant color will be pure white 
	{
		color =  vec4(1.0);
		return;
	}
	
	// For foliage ^_^
	int index = int (m_materialindex);
	vec3 alpha = texture(alpha_texture , vec3(tcord,index)).xyz;
	//if(alpha.r <= 0.06)
		//discard;

	vec4 coordinate = u_ProjectionView * m_pos;
	coordinate.xyz /= coordinate.w;
	coordinate.xyz = coordinate.xyz*0.5 + 0.5;

	vec4 position = texture(GPosition,coordinate.xy);// sample the position map

	vec2 noiseScale = vec2(ScreenWidth/4.0 , ScreenHeight/4.0);
	float occlusion = 0.0;
	vec3 FragPos = position.xyz;
	vec3 RandomVec = texture(noisetex , coordinate.xy*noiseScale).xyz;
	vec3 normal = m_Normal;

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