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
	vec3 envColor = texture(env,direction).xyz;
	//envColor = envColor / (envColor + vec3(1.0));
	vec3 mapped = vec3(1.0) - exp(-envColor * 3.0);//exposure
	mapped = pow(mapped, vec3(1.0/2.2)); 

	color= vec4(mapped,1.0);
}