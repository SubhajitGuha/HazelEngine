#shader vertex
#version 450 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 cord;

out vec2 tcord;
uniform mat4 u_ProjectionView;
uniform mat4 u_ModelTransform;

void main()
{
	tcord = cord;
	gl_Position = u_ProjectionView * u_ModelTransform * vec4(pos ,1.0);
	
}
#shader fragment
#version 450 core
layout (location = 0) out vec4 color;

in vec2 tcord;
uniform sampler2D u_Texture;
uniform vec4 u_color;
void main()
{
	color= texture(u_Texture,tcord) * u_color;
}