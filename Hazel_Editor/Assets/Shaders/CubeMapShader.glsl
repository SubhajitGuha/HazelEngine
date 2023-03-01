#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 dir;

out vec3 direction;

void main()
{
	direction = vec3(dir.x,dir.y,dir.z);
	gl_Position = pos;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

in vec3 direction; 
uniform samplerCube env;

void main()
{
	color= texture(env,direction);
}