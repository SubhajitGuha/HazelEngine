#shader vertex
#version 410
layout (location = 0)in vec3 pos;

out vec3 locPos;
uniform mat4 u_ProjectionView;

void main()
{
	locPos = pos;
	gl_Position = u_ProjectionView * vec4(pos,1.0);
}

#shader fragment
#version 410 core
layout (location = 0)out vec4 color;

in vec3 locPos;
uniform samplerCube environmentMap;

const float PI = 3.14159265359;
void main()
{
	vec3 normal = normalize(locPos);
	vec3 up = vec3(0.0,1.0,0.0);
	vec3 right = normalize(cross(up,normal));
	up = normalize(cross(normal,right));

	vec3 irradiance = vec3(0.0);
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
    
            irradiance += textureLod(environmentMap, sampleVec,0).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    color = vec4(irradiance,1.0);
}
