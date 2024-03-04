#shader vertex
#version 410 core
#define PI 3.14159265359

layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 cord;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 Tangent;
layout (location = 4) in vec3 BiTangent;
layout (location = 5) in mat4 instance_mm;

out vec2 tcord;
out vec3 objSpacePos;
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
uniform int applyGradientMask;
uniform int enableWind;

uniform vec3 u_BoundsExtent;
float amplitude=2.0;
float wsAmplitude=0.3;

void main()
{	
	mat4 wsGrass = u_Model * instance_mm;
	vec4 wsVertexPos = wsGrass * pos;
	objSpacePos = pos.xyz;

	vec3 origin = vec3(wsGrass[3][0],wsGrass[3][1],wsGrass[3][2]);
	//wind system
	float factor = pos.y/u_BoundsExtent.y;	//gradient calculated at object space by dividing with the bounds

	if(factor<0.05)
		factor = 0;
	
	vec2 size = textureSize(Noise,0);
	vec2 coord = mod(origin.xz,size);
	coord /= size;

	if(enableWind == 1)
	{
		float noise = texture(Noise , coord ).r;
		float rotVal = (cos(u_Time * PI * noise) * cos(u_Time *0.2* PI)) * wsAmplitude;
		float ws_rotVal = (cos(u_Time * PI * 0.8) * cos(u_Time *0.2* PI)) * wsAmplitude + sin(PI*u_Time*noise)*1;
		
		wsVertexPos.x += ws_rotVal * factor;
		wsVertexPos.z += rotVal * factor;
	}
	vec4 clip_space = u_ProjectionView * wsVertexPos;
	m_curPos = clip_space;
	m_oldPos = u_oldProjectionView * wsVertexPos; //required for the velocity buffer
	gl_Position = clip_space;

	if(applyGradientMask == 1)
		m_VertexColor = clamp(factor,0.2,1.0) * m_color.xyz;
	else
		m_VertexColor = m_color.xyz;
		
	tcord = cord;
	m_Normal = normalize(mat3(u_View * wsGrass ) * Normal );
	m_Tangent = normalize(mat3(u_View * wsGrass ) * Tangent );
	m_BiTangent = normalize(mat3(u_View * wsGrass ) * BiTangent );

	m_pos = u_View * wsVertexPos;

}

#shader fragment
#version 410 core
layout (location = 0) out vec4 gNormal;
layout (location = 1) out vec4 gVelocity;
layout (location = 2) out vec4 gColor;
layout (location = 3) out vec4 gRoughnessMetallic;

//pbr mapping
//the u_Roughness texture contains "opacity map" on R-channel "Roughness map" on G-channel and "Ambient occlusion" on B-channel

in vec4 m_pos;
in vec3 objSpacePos;
in vec4 m_curPos; //current clip-space position
in vec4 m_oldPos; //previous clip-space position
in vec3 m_Normal;
in vec3 m_Tangent;
in vec3 m_BiTangent;
in vec3 m_VertexColor;
in vec2 tcord;

const float g_HashedScale = 1.0;

uniform sampler2D u_Albedo;
uniform sampler2D u_Roughness;
uniform sampler2D u_NormalMap;
//uniform vec4 m_color;

//PBR properties
uniform float Roughness;
uniform float Metallic;

vec3 PBR_Color = vec3(0.0);

float alpha = Roughness; //Roughness value
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
	
	oldPos /= oldPos.w;
	oldPos.xy = (oldPos.xy+1.0) * 0.5;
	
	return (curPos - oldPos).xy;
}

float hash(vec2 val)
{
	return fract(1.0e4 * sin(17.0*val.x + 0.1*val.y) * 
			(0.1+abs(sin(13.0*val.y + val.x))));
}
float hash3D(vec3 val)
{
	return hash(vec2(hash(val.xy),val.z));
}
//hached alpha testing, code used from https://developer.download.nvidia.com/assets/gameworks/downloads/regular/GDC17/RealTimeRenderingAdvances_HashedAlphaTesting_GDC2017_FINAL.pdf?t=eyJscyI6ImdzZW8iLCJsc2QiOiJodHRwczovL3d3dy5nb29nbGUuY28uaW4vIn0=
float HashedAlphaThreshold()
{
	float maxDeriv = max(length(dFdx(objSpacePos)), length(dFdy(objSpacePos)));
	float pixScale = 1.0/(g_HashedScale * maxDeriv);

	vec2 pixScales = vec2(exp2(floor(log2(pixScale))), exp2(ceil(log2(pixScale))) );

	vec2 alpha = vec2(hash3D(floor(pixScales.x*objSpacePos)), 
					hash3D(floor(pixScales.y*objSpacePos)));

	float lerpFactor = fract(log2(pixScale));

	float x = (1.0-lerpFactor)*alpha.x + lerpFactor*alpha.y;

	float a = min(lerpFactor, 1.0-lerpFactor);
	vec3 cases = vec3(x*x/(2.0*a*(1.0-a)), 
					(x-0.5*a)/(1.0-a), 
					1.0-((1.0-x)*(1.0-x)/(2.0*a*(1.0-a))) );
	
	float alpha_f = (x<(1.0-a)) ? 
					((x<a) ? cases.x : cases.y): 
					cases.z;

	alpha_f = clamp(alpha_f, 1.0e-6, 1.0);

	return alpha_f;
}

float ao = 1.0;
void main()
{
	vec4 albedo = texture(u_Albedo, tcord);
	float alpha = albedo.a;	
	if(alpha < HashedAlphaThreshold())
		discard;	
	gNormal = vec4(NormalMapping(),1.0);
	gVelocity = vec4(CalculateVelocity(m_curPos,m_oldPos),0,1.0);
	gColor = vec4(GammaCorrection(albedo.rgb * m_VertexColor), 1.0);
	gRoughnessMetallic = vec4(texture(u_Roughness,tcord).r*Roughness,Metallic,1,1.0);
}