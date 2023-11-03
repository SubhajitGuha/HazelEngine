#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 dir;

out vec3 direction;

void main()
{
	direction = -dir.xyz;
	gl_Position = pos;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

in vec3 direction; 
uniform samplerCube env;

void main()
{
	vec3 envColor = textureLod(env,normalize(direction),0.0).xyz;
	//envColor = envColor / (envColor + vec3(1.0));
	//vec3 mapped = vec3(1.0) - exp(-envColor * 3.0);//exposure
	//mapped = pow(mapped, vec3(1.0/2.2)); 
	//mapped = clamp(mapped,0.0,1.0);
	color= vec4(envColor,1.0);
}