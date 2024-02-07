#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 cord;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 Tangent;
layout (location = 4) in vec3 BiTangent;
layout (location = 5) in mat4 instance_mm;

out vec2 tcord;
out vec4 m_pos;
out vec4 m_curPos; //current clip-space position
out vec4 m_oldPos; //previous clip-space position
out vec3 m_Normal;
out vec3 m_Tangent;
out vec3 m_BiTangent;
out vec3 m_VertexColor;

uniform mat4 u_ProjectionView;
uniform mat4 u_oldProjectionView;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat4 u_Model;
uniform vec3 u_cameraPos;
uniform float u_Time;
uniform vec4 m_color;
uniform sampler2D Noise;

float amplitude=0;
float wsAmplitude=0.0;

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

mat4 CreateScaleMatrix(float scale)
{
	return mat4(
	scale,0,0,0,
	0,scale,0,0,
	0,0,scale,0,
	0,0,0,1
	);
}
void main()
{	
	mat4 wsGrass = u_Model * instance_mm;
	vec4 wsVertexPos = wsGrass * pos;
	vec4 vsVertexPos = u_View * wsVertexPos;
	vec3 plane_normal = normalize(wsVertexPos.xyz - u_cameraPos);

	vec3 origin = vec3(wsGrass[3][0],wsGrass[3][1],wsGrass[3][2]);

	//wind system
	float factor = distance(wsVertexPos.xyz , origin)/3;	//bottom part of foliage is not affected by wind
	if(factor<0)
		factor = 0;
	
	vec3 coord = mod(abs(wsVertexPos.xyz),256);
	coord/=vec3(256); //256 is the size of noise texture

	vec3 noise = texture(Noise , coord.xz*2 ).rgb*2;
	vec3 rotVal = vec3(sin(u_Time + noise.r)*0.5+0.5+1) *factor*amplitude;
	vec3 ws_rotVal = vec3(noise.r*sin(u_Time)*0.5+0.5) *factor*wsAmplitude;

	mat4 ws_rot = CreateRotationMat(0, 0, ws_rotVal.y);
	mat4 rot = CreateRotationMat(0, rotVal.y, 0);

	float val = texture(Noise,coord.xz*10).r;

	//gl_Position = u_ProjectionView * ws_rot * wsGrass * rot * CreateScaleMatrix(val*2) * pos;
	vec4 clip_space = u_ProjectionView * ws_rot * wsGrass * rot * pos;
	m_curPos = clip_space;
	m_oldPos = u_oldProjectionView * ws_rot * wsGrass * rot * pos; //required for the velocity buffer
	gl_Position = clip_space;

	m_VertexColor = clamp(factor,0.0,1.0) * m_color.xyz;
	//m_VertexColor = m_color.xyz;
	tcord = cord;
	m_Normal = normalize(mat3(u_View * wsGrass ) * Normal );
	m_Tangent = normalize(mat3(u_View * wsGrass ) * Tangent );
	m_BiTangent = normalize(mat3(u_View * wsGrass ) * BiTangent );

	m_pos = u_View * wsGrass * pos;

}

#shader fragment
#version 410 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gColor;
layout (location = 3) out vec4 gRoughnessMetallic;
layout (location = 4) out vec4 gVelocity;

//pbr mapping
//the u_Roughness texture contains "opacity map" on R-channel "Roughness map" on G-channel and "Ambient occlusion" on B-channel

in vec4 m_pos;
in vec4 m_curPos; //current clip-space position
in vec4 m_oldPos; //previous clip-space position
in vec3 m_Normal;
in vec3 m_Tangent;
in vec3 m_BiTangent;
in vec3 m_VertexColor;
in vec2 tcord;


uniform samplerCube diffuse_env;
uniform samplerCube specular_env;

uniform sampler2D u_Albedo;
uniform sampler2D u_Roughness;
uniform sampler2D u_NormalMap;
//uniform vec4 m_color;

//PBR properties
uniform float Roughness;
uniform float Metallic;

vec3 PBR_Color = vec3(0.0);

float alpha = Roughness; //Roughness value
const float PI = 3.14159265359;
#define MAX_MIP_LEVEL 28
int level = 3; // cascade levels
float NdotL = 1.0;


vec3 NormalMapping()
{
	vec3 normal = texture(u_NormalMap , tcord).rgb;
	normal = normal*2.0 - 1.0;
	mat3 TBN = mat3(m_Tangent , m_BiTangent , m_Normal);
	if(normal == vec3(1.0))
		return normalize(m_Normal);
	else
		return normalize(TBN * normal);
}
vec3 GammaCorrection(in vec3 color)
{
	return pow(color, vec3(2.2)); //Gamma space
}

vec2 CalculateVelocity(in vec4 curPos,in vec4 oldPos)
{
	curPos /= curPos.w;
	curPos.xy = (curPos.xy+1.0) * 0.5; //convert to 0-1
	//curPos.y = 1.0 - curPos.y;

	oldPos /= oldPos.w;
	oldPos.xy = (oldPos.xy+1.0) * 0.5;
	//oldPos.y = 1.0 - oldPos.y;

	return (curPos - oldPos).xy;
}

float ao = 1.0;
void main()
{
	vec4 albedo = texture(u_Albedo, tcord);
	mat4x4 thresholdMatrix = mat4x4(
	
		1.0 / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0,
		13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0,
		4.0 / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0,
		16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0
	);
	float alpha = albedo.a;
	float val = thresholdMatrix[int(gl_FragCoord.x) % 4][int(gl_FragCoord.y) % 4];
	if(alpha < 0.5f)
		discard;
	gPosition = vec4(m_pos.xyz,1.0);
	gNormal = vec4(NormalMapping(),1.0);
	gColor = vec4(GammaCorrection(albedo.rgb * m_VertexColor), 1.0);
	gRoughnessMetallic = vec4(texture(u_Roughness,tcord).r*Roughness,Metallic,1,1.0);
	gVelocity = vec4(CalculateVelocity(m_curPos,m_oldPos),0,1.0);
}