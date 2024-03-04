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

    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This shows 5 square areas that are being sampled. But some of them overlap,
    // so to have an energy preserving downsample we need to make some adjustments.
    // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
    // contribute 0.5 to the final color output. The code below is written
    // to effectively yield this sum. We get:
    // 0.125*5 + 0.03125*4 + 0.0625*4 = 1

    vec4 calculated_color = vec4(0);
    calculated_color = e*0.125;
    calculated_color += (a+c+g+i)*0.03125;//0.125
    calculated_color += (b+d+f+h)*0.0625;//0.25
    calculated_color += (j+k+l+m)*0.125;//0.5

    color = vec4(calculated_color.xyz,1.0);
}