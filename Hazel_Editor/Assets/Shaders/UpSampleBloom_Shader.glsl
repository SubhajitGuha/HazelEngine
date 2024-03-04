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
uniform float FilterRadius;
uniform vec3 ImageRes;

void main()
{
	float x = (1.0/ImageRes.x) * FilterRadius;
	float y = (1.0/ImageRes.y) * FilterRadius;

	vec4 a = texture(inputImage , vec2(tcord.x - x, tcord.y + y));
	vec4 b = texture(inputImage , vec2(tcord.x , tcord.y + y));
	vec4 c = texture(inputImage , vec2(tcord.x + x, tcord.y + y));

	vec4 d = texture(inputImage , vec2(tcord.x - x, tcord.y));
	vec4 e = texture(inputImage , vec2(tcord.x , tcord.y));
	vec4 f = texture(inputImage , vec2(tcord.x + x, tcord.y));

	vec4 g = texture(inputImage , vec2(tcord.x - x, tcord.y - y));
	vec4 h = texture(inputImage , vec2(tcord.x , tcord.y - y));
	vec4 i = texture(inputImage , vec2(tcord.x + x, tcord.y - y));
	//weighted box Filter
	//  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
	vec4 calculated_color;
	calculated_color = e * 4.0;
	calculated_color += (b+d+f+h) * 2.0;
	calculated_color += (a+c+g+i)* 1.0;
	calculated_color *= 1.0/16.0; 

	color = vec4(calculated_color.xyz,1.0);

}