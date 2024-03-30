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
out vec3 objSpacePos;
out vec3 m_Normal;
out vec3 m_Tangent;
out vec3 m_BiTangent;
out vec3 m_VertexColor;

uniform mat4 LightProjection;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat4 u_Model;
uniform vec3 u_cameraPos;
uniform float u_Time;
uniform vec4 m_color;
uniform sampler2D Noise;
float amplitude=40;
float wsAmplitude=0.6;

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
	vec4 wsGrass = u_Model * instance_mm * pos;
	objSpacePos = wsGrass.xyz/wsGrass.w;
	gl_Position = LightProjection * wsGrass ;
	tcord = cord;
}

#shader fragment
#version 410 core

in vec2 tcord;
in vec3 objSpacePos;
const float g_HashedScale = 1.0;
uniform sampler2D u_Albedo; // multiple material slots can be present so a texture array is used

float hash(vec2 val)
{
	return fract(1.0e4 * sin(17.0*val.x + 0.1*val.y) * 
			(0.1+abs(sin(13.0*val.y + val.x))));
}
float hash3D(vec3 val)
{
	return hash(vec2(hash(val.xy),val.z));
}

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

void main()
{
	vec4 albedo = texture(u_Albedo, tcord);
	
	float alpha = albedo.a;
	//if(alpha < 0.8)
		//discard;
}