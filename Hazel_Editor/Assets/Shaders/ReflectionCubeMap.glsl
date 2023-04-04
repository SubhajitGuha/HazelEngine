#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 dir;

varying vec3 direction;

void main()
{
	direction = dir.xyz/dir.w;
	gl_Position = pos;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

varying vec3 direction;

uniform samplerCube env;

const float PI = 3.14159265359;

vec3 GetIrradiance()
{
	vec3 normal = normalize(-direction.xyz);
	vec3 irradiance = vec3(0.0);
	
	vec3 up    = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, normal));
	up         = normalize(cross(normal, right));
	
	float sampleDelta = 0.025;
	float nrSamples = 0.0; 
	for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
	    for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
	    {
	        // spherical to cartesian (in tangent space)
	        vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
	        // tangent space to world
	        vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; 
	
	        irradiance += texture(env, sampleVec).rgb * cos(theta) * sin(theta);
	        nrSamples++;
	    }
	}
	irradiance = PI * irradiance * (1.0 / float(nrSamples));
	return irradiance;
}

void main()
{
	vec3 envColor = GetIrradiance();
	envColor = envColor / (envColor + vec3(1.0));
	//envColor = vec3(1.0) - exp(-envColor * 3.0);//exposure
	envColor = pow(envColor, vec3(1.0/2.2)); 

	color= vec4(envColor,1.0);
}