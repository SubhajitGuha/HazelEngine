#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 cord;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 Tangent;
layout (location = 4) in vec3 BiTangent;
layout (location = 5) in float materialindex;

out vec2 tcord;
out vec4 m_pos;
flat out float m_materialindex;

uniform mat4 u_ProjectionView;
uniform mat4 u_Model;

void main()
{
	gl_Position = u_ProjectionView * u_Model * pos;
	m_pos = u_Model * pos;
	tcord = cord;
	m_materialindex = materialindex;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

in vec2 tcord;
in vec4 m_pos;
flat in float m_materialindex;

uniform sampler2D SSAOtex;
uniform mat4 u_ProjectionView;

//For foliages only
uniform sampler2DArray alpha_texture; //for foliages
uniform int isFoliage;//For foliages

void main()
{
	// For foliage ^_^
	int index = int (m_materialindex);
	vec3 alpha = texture(alpha_texture , vec3(tcord,index)).xyz;
	if(isFoliage == 1 && alpha.r <= 0.06)
		discard;

	vec4 coordinate = u_ProjectionView * m_pos;
	coordinate.xyz /= coordinate.w;
	coordinate.xyz = coordinate.xyz*0.5 + 0.5;

	float pixel_dimension = 1.0/textureSize(SSAOtex,0).x ;
	float value = 0.0;

	for(int i=-2; i<=2; i++)
	{
		for(int j=-2; j<=2; j++)
		{
			vec2 offset = vec2(i,j) * pixel_dimension;
			value += texture(SSAOtex,coordinate.xy + offset).r;
		}
	}
	color =  vec4(vec3(value/25.0),1.0);
}