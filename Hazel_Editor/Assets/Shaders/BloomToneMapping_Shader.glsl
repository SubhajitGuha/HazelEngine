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

void main()
{
	const float gamma = 0.8;
    vec3 hdrColor = texture(inputImage, tcord).rgb;//masked out only the bright parts
	vec3 Original = texture(OriginalImage, tcord).rgb;

	hdrColor *= exposure/(1.0 + hdrColor / exposure);
	hdrColor = pow(hdrColor, vec3(1.0 / 2.2));

	 Original = mix(Original , Original + hdrColor * BloomAmount,hdrColor.xyz);

	color = vec4(Original,1.0);
}