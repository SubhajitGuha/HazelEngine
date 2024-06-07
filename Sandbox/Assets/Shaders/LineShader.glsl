#shader vertex
#version 460 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 color;

out vec4 m_color;
uniform mat4 u_ProjectionView;
void main()
{
	m_color = color;
	gl_Position = u_ProjectionView * pos;
}

#shader fragment
#version 460 core
layout (location = 0) out vec4 col;

in vec4 m_color;

void main()
{
	col = m_color;
}