#shader vertex
#version 450 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 cord;

out vec2 tcord;
uniform mat4 m_ProjectionView;
uniform mat4 m_ModelTransform;
void main()
{
	tcord = cord;
	gl_Position = m_ProjectionView * m_ModelTransform * vec4(pos ,1.0);
	
}
#shader fragment
#version 450 core
layout (location = 0) out vec4 color;

in vec2 tcord;
uniform sampler2D u_Texture;
void main()
{
	color= texture(u_Texture,tcord);
}