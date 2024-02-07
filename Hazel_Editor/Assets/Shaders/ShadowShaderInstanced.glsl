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
	mat4 wsGrass = u_Model * instance_mm;
	gl_Position = LightProjection * wsGrass * pos;
	tcord = cord;
}

#shader fragment
#version 410 core

in vec2 tcord;
uniform sampler2D u_Albedo; // multiple material slots can be present so a texture array is used

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
	if(alpha < val)
		discard;
}