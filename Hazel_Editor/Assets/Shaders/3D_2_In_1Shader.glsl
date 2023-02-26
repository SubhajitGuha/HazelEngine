#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 cord;
layout (location = 2) in vec4 color;
layout (location = 3) in vec3 Normal;
layout (location = 4) in float slotindex;

out vec2 tcord;
out vec4 m_pos;
out vec4 m_color;
out vec3 m_Normal;
flat out float m_slotindex;

uniform mat4 u_ProjectionView;
uniform mat4 u_ModelTransform;

void main()
{
	gl_Position = u_ProjectionView * pos;
	m_color = color;
	m_slotindex = slotindex;
	tcord = cord;
	m_Normal = Normal;
	m_pos = pos;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

in vec4 m_color;
in vec4 m_pos;
in vec3 m_Normal;
flat in float m_slotindex;
in vec2 tcord;

uniform sampler2D u_Texture[32];
uniform vec4 u_color;
uniform vec3 LightPosition;

void main()
{
	vec3 LightDirection = normalize(LightPosition - vec3(m_pos.x,m_pos.y,m_pos.z));
	float brightness = dot(LightDirection , m_Normal);
	vec4 f_color = m_color * vec4(brightness,brightness,brightness,1);
	int index = int (m_slotindex);
	color= texture(u_Texture[index],tcord) * f_color ;
}