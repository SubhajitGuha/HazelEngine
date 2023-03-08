#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 cord;
layout (location = 2) in vec4 color;
layout (location = 3) in vec3 Normal;
layout (location = 4) in float slotindex;

uniform mat4 LightProjection; //matrixshadow is the model light projection but converted to 0-1 range

void main()
{
	gl_Position = LightProjection * pos;
}

#shader fragment
#version 410 core
//layout (location = 0) out vec4 color;


void main()
{
}