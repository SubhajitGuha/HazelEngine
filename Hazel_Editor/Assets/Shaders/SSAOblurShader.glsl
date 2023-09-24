#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 cord;

out vec2 tcord;

uniform mat4 u_ProjectionView;
uniform mat4 u_Model;

void main()
{
	gl_Position =  pos;
	tcord = cord.xy;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

in vec2 tcord;

uniform sampler2D SSAOtex;

void main()
{
	float pixel_dimension = 1.0/textureSize(SSAOtex,0).x ;
	float value = 0.0;

	for(int i=-2; i<=2; i++)
	{
		for(int j=-2; j<=2; j++)
		{
			vec2 offset = vec2(i,j) * pixel_dimension;
			value += texture(SSAOtex,tcord + offset).r;
		}
	}
	color =  vec4(vec3(value/25.0),1.0);
}