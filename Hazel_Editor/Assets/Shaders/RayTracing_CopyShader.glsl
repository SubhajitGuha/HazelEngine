//#shader vertex
#version 430 core

layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 coord;

out vec2 tcoord;
void main()
{
	gl_Position = pos;
	tcoord = coord.xy;
}

//#shader fragment
#version 430 core

layout (location = 0)out vec4 color;

uniform sampler2D InputTexture;
in vec2 tcoord;
void main()
{
	color = vec4(texture(InputTexture,tcoord).rgb,1.0);
}