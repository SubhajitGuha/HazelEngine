#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 cord;
layout (location = 2) in vec4 color;
layout (location = 3) in float slotindex;

out vec2 tcord;
out vec4 m_color;
out float m_slotindex;

uniform mat4 u_ProjectionView;
uniform mat4 u_ModelTransform;

void main()
{
	m_color = color;
	m_slotindex = slotindex;
	tcord = cord;
	gl_Position = u_ProjectionView * pos;
	
}
#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

in vec4 m_color;
in float m_slotindex;
in vec2 tcord;

uniform sampler2D u_Texture[32];
uniform vec4 u_color;

void main()
{
	int index = int (m_slotindex);
	color= texture(u_Texture[index],tcord) * m_color;
}