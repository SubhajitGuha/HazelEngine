#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 color;

out vec4 m_color;

void main()
{
	gl_Position = pos;
	m_color = color;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 col;

in vec4 m_color;

void main(){
	col= m_color;
}