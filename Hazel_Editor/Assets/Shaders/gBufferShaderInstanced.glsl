#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 cord;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 Tangent;
layout (location = 4) in vec3 BiTangent;
layout (location = 5) in float materialindex;
layout (location = 6) in mat4 instance_mm;

uniform mat4 u_View; //matrixshadow is the model light projection but converted to 0-1 range
uniform mat4 u_Model;
uniform mat4 u_Projection;
uniform sampler2D Noise;
uniform float u_Time;
float amplitude=40;
float wsAmplitude=0.6;

out vec4 m_Pos;
out vec2 tcoord;
flat out float m_materialindex;

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
	vec3 rotVal = vec3(sin(u_Time + noise.r)*0.5+0.5 + 1.0) *factor*amplitude;
	vec3 ws_rotVal = vec3(noise.r*sin(u_Time)*0.5+0.5) *factor*wsAmplitude;

	mat4 ws_rot = CreateRotationMat(0, 0, ws_rotVal.y);
	mat4 rot = CreateRotationMat(0, rotVal.y, 0);

	float val = texture(Noise,coord.xz*10).r;

	gl_Position = u_Projection * u_View * ws_rot * wsGrass * rot * CreateScaleMatrix(val*2) * pos;

	m_Pos = u_View * ws_rot * wsGrass * rot * CreateScaleMatrix(val*2) * pos;
	m_materialindex = materialindex;
	tcoord = cord;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

in vec4 m_Pos;
in vec2 tcoord;
flat in float m_materialindex;

uniform sampler2DArray u_Alpha; // multiple material slots can be present so a texture array is used

void main()
{
	int index = int (m_materialindex);
	vec3 alpha = texture(u_Alpha , vec3(tcoord , index)).rgb;
	
	//if(alpha.r <=0.06 ) // check if the tex value is less than a certain threshold then discard
		//discard;
	color = m_Pos;
}