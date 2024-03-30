#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 dir;

out vec2 tcord;
out vec3 viewDir;

void main()
{
	gl_Position = pos;
	//tcord = cord.xy;

	viewDir = dir.xyz;
    tcord = pos.xy*0.5+0.5;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

in vec2 tcord;
in vec3 viewDir;

uniform vec3 sun_direction;
uniform sampler2DArray Sky_Gradient;


vec3 getSun(float sunViewDot)
{
    float sunSize = 0.03;    
    return vec3(1.0,0.8,0.8)*step(1.0- sunSize * sunSize, sunViewDot);
}

vec3 getSky(vec2 uv)
{
    vec3 _SunDir = normalize(-sun_direction);
    vec3 _ViewDir = normalize(viewDir);
    float sunViewDot = dot(_SunDir, _ViewDir);
    float sunZenithDot = _SunDir.y;
    float viewZenithDot = _ViewDir.y;
    
    float sunViewDot01 = (sunViewDot + 1.0) * 0.5;
    float sunZenithDot01 = (sunZenithDot + 1.0) * 0.5;

    vec3 skyColor = texture(Sky_Gradient,vec3(sunZenithDot01,0.5,0)).rgb;

    vec3 viewZenithColor = texture(Sky_Gradient, vec3(sunZenithDot01,0.5,1)).rgb;
    float vzMask = pow(clamp(1.0 - viewZenithDot,0.0,1.0), 8.0);

    vec3 sunViewColor = texture(Sky_Gradient, vec3(sunZenithDot01,0.5,1)).rgb;
    float svMask = pow(clamp(sunViewDot,0.0,1.0), 256.0);

    return skyColor + getSun(sunViewDot) + vzMask * viewZenithColor + svMask*sunViewColor;
}

  void main()
  {
    color = vec4(getSky(tcord).rgb,1.0);
  }