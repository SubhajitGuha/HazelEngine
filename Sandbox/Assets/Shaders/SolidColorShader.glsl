#shader vertex
#version 460 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 col;

out vec4 m_color;
uniform mat4 m_ProjectionView;
uniform mat4 m_ModelTransform;
void main()
{
	m_color = col;
	gl_Position = m_ProjectionView * m_ModelTransform * vec4(pos ,1.0);
}

#shader fragment
#version 460 core
layout (location = 0)out vec4 col;
uniform vec4 m_color;
void main(){
	col= m_color;
}