#shader vertex
#version 410 core

void main()
{
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
	envColor = vec3(1.0) - exp(-envColor * 5.0);//exposure
	envColor = pow(envColor, vec3(1.0/2.2)); 

	color= vec4(envColor,1.0);
}