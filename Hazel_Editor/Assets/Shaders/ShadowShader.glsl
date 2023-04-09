#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 cord;
layout (location = 2) in vec4 color;
layout (location = 3) in vec3 Normal;
layout (location = 4) in vec3 Tangent;
layout (location = 5) in vec3 BiTangent;
layout (location = 6) in float materialindex;

uniform mat4 LightProjection; //matrixshadow is the model light projection but converted to 0-1 range
flat out float m_materialindex;
out vec2 m_tcoord;

void main()
{
	m_tcoord = cord;
	m_materialindex = materialindex;
	gl_Position = LightProjection * pos;
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