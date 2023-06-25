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
uniform vec3 ImageRes;

void main()
{
	vec2 pixelSize = 1.0/ImageRes.xy;
	float x = pixelSize.x;
	float y = pixelSize.y;

	vec4 a = texture(inputImage, vec2(tcord.x - 2*x,tcord.y + 2*y));
	vec4 b = texture(inputImage, vec2(tcord.x, tcord.y + 2*y));
    vec4 c = texture(inputImage, vec2(tcord.x + 2*x, tcord.y + 2*y));
       
    vec4 d = texture(inputImage, vec2(tcord.x - 2*x, tcord.y));
    vec4 e = texture(inputImage, vec2(tcord.x,       tcord.y));
    vec4 f = texture(inputImage, vec2(tcord.x + 2*x, tcord.y));
       
    vec4 g = texture(inputImage, vec2(tcord.x - 2*x, tcord.y - 2*y));
    vec4 h = texture(inputImage, vec2(tcord.x,       tcord.y - 2*y));
    vec4 i = texture(inputImage, vec2(tcord.x + 2*x, tcord.y - 2*y));
       
    vec4 j = texture(inputImage, vec2(tcord.x - x, tcord.y + y));
    vec4 k = texture(inputImage, vec2(tcord.x + x, tcord.y + y));
    vec4 l = texture(inputImage, vec2(tcord.x - x, tcord.y - y));
    vec4 m = texture(inputImage, vec2(tcord.x + x, tcord.y - y));


    vec4 calculated_color = vec4(0);
    calculated_color = e*0.125;
    calculated_color += (a+c+g+i)*0.03125;
    calculated_color += (b+d+f+h)*0.0625;
    calculated_color += (j+k+l+m)*0.125;

    color = vec4(calculated_color.xyz,1.0);
}