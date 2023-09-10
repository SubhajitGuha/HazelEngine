#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 cord;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 Tangent;
layout (location = 4) in vec3 BiTangent;
layout (location = 5) in float materialindex;
layout (location = 6) in mat4 instance_mm;

uniform mat4 LightProjection; //matrixshadow is the model light projection but converted to 0-1 range
flat out float m_materialindex;
uniform float u_Time;
uniform mat4 u_Model;
uniform mat4 u_Projection;
uniform sampler2D Noise;

float amplitude=70;
out vec2 m_tcoord;

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

	vec3 dir = texture(Noise , coord.xz * u_Time).rgb;
	vec3 rotVal = (vec3(sin(coord.x + coord.y + u_Time + dir.r)) ) *factor*amplitude;
	mat4 rot = CreateRotationMat(0, rotVal.y, 0);

	float val = texture(Noise,coord.xz*10).r;

	gl_Position = LightProjection * wsGrass * rot* CreateScaleMatrix(val*2) * pos;
}

#shader fragment
#version 410 core

flat in float m_materialindex;
in vec2 m_tcoord;
uniform sampler2DArray u_Alpha; // multiple material slots can be present so a texture array is used
uniform int isFoliage; // check for foliage

void main()
{
	int index = int (m_materialindex);
	vec3 alpha = texture(u_Alpha , vec3(m_tcoord , index)).rgb;
	
	if(isFoliage == 1 && alpha.r <=0.06 ) // check if the tex value is less than a certain threshold then discard
		discard;
}