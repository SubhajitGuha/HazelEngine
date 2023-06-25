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
uniform float BightnessThreshold;

mat4 contrastMatrix( float contrast )
{
	float t = ( 1.0 - contrast ) / 2.0;
    
    return mat4( contrast, 0, 0, 0,
                 0, contrast, 0, 0,
                 0, 0, contrast, 0,
                 t, t, t, 1 );

}

void main()
{
	vec4 FragColor = texture(inputImage , tcord);

	//vec4 HighContrast = FragColor; 

	float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
	if(brightness > BightnessThreshold)
		color = vec4(FragColor.xyz , 1.0);
	else
		color = vec4(0,0,0,1.0);

}