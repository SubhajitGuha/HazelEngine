#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 cord;
layout (location = 2) in vec4 color;
layout (location = 3) in vec3 Normal;
layout (location = 4) in vec3 Tangent;
layout (location = 5) in vec3 BiTangent;
layout (location = 6) in float slotindex;

out vec2 tcord;
out vec4 m_pos;
out vec4 m_color;
out vec3 m_Normal;
out vec3 m_Tangent;
out vec3 m_BiTangent;
flat out float m_slotindex;

uniform mat4 u_ProjectionView;
uniform mat4 u_View;

void main()
{
	gl_Position = u_ProjectionView * pos;
	m_color = color;
	m_slotindex = slotindex;
	tcord = cord;
	m_Normal = Normal;
	m_Tangent = Tangent;
	m_BiTangent = BiTangent;
	m_pos = u_View * pos;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

in vec4 m_color;
in vec4 m_pos;
in vec3 m_Normal;
in vec3 m_Tangent;
in vec3 m_BiTangent;
flat in float m_slotindex;
in vec2 tcord;


void main()
{
	color = m_pos;
}