#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 cord;

out vec2 tcord;
out vec4 m_pos;

void main()
{
	gl_Position = pos;
	tcord = cord.xy;
	m_pos = pos;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

in vec2 tcord;
in vec4 m_pos;

uniform sampler2D inputImage;
uniform sampler2D OriginalImage;
uniform float exposure;
uniform float BloomAmount;

vec3 ACESFilm(vec3 x)
{
	float a = 2.51;
	float b = 0.03;
	float c = 2.43;
	float d = 0.59;
	float e = 0.14;
	return clamp((x*(a*x+b))/(x*(c*x+d)+e),0.0,1.0);
}

void main()
{
	const float gamma = 0.8;
    vec3 hdrColor = texture(inputImage, tcord).rgb;//masked out only the bright parts
	vec3 Original = texture(OriginalImage, tcord).rgb;

	hdrColor *= exposure/(1.0 + hdrColor / exposure);
	Original = mix(Original , Original + hdrColor * BloomAmount,hdrColor.xyz);

	Original = ACESFilm(Original);//needs to be in a seperate shader
	Original = pow(Original, vec3(1.0 / 2.2));

	color = vec4(Original,1.0);
}