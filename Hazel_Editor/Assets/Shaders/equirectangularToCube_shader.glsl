#shader vertex
#version 410 core
layout (location = 0)in vec3 pos;

out vec3 locPos;
uniform mat4 u_ProjectionView;
void main()
{
	locPos = pos;
	gl_Position = u_ProjectionView * vec4(pos,1);
}

#shader fragment
#version 410 core
layout (location = 0)out vec4 color;

in vec3 locPos;
uniform sampler2D hdrTexture;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(locPos));
    vec3 envColor = texture(hdrTexture, uv).rgb;
    vec3 mapped = vec3(1.0) - exp(-envColor * 1.0);//exposure
	//mapped = pow(mapped, vec3(1.0/2.2)); 
	//mapped = clamp(mapped,0.0,1.0);
    color = vec4(mapped,1.0);
}