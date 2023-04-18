#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 cord;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 Tangent;
layout (location = 4) in vec3 BiTangent;
layout (location = 5) in float materialindex;

out vec2 tcord;
out vec4 m_pos;
out vec3 m_Tangent;
out vec3 m_BiTangent;
flat out float m_materialindex;

uniform mat4 u_ProjectionView;
uniform mat4 u_Model;
uniform mat4 u_View;

void main()
{
	gl_Position = u_ProjectionView * u_Model * pos;
	m_materialindex = materialindex;
	tcord = cord;
	m_Tangent = Tangent;
	m_BiTangent = BiTangent;
	m_pos = u_View * u_Model * pos;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

in vec4 m_pos;
in vec3 m_Tangent;
in vec3 m_BiTangent;
flat in float m_materialindex;
in vec2 tcord;

uniform sampler2DArray alpha_texture;
uniform int isFoliage;//For foliages

void main()
{
	//for foliage only
	int index = int (m_materialindex);
	vec3 alpha = texture(alpha_texture , vec3(tcord , index)).rgb;
	if(isFoliage == 1 && alpha.r <=0.06 ) // check if the tex value is less than a certain threshold then discard
		discard;

	color = m_pos;
}