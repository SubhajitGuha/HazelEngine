#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 cord;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 Tangent;
layout (location = 4) in vec3 BiTangent;

uniform mat4 LightProjection; //matrixshadow is the model light projection but converted to 0-1 range
uniform mat4 u_Model;

out vec2 m_tcoord;

void main()
{
	m_tcoord = cord;
	gl_Position = LightProjection * u_Model * pos;
}

#shader fragment
#version 410 core

in vec2 m_tcoord;
uniform sampler2D u_Alpha; // multiple material slots can be present so a texture array is used
uniform int isFoliage; // check for foliage

void main()
{
	vec3 alpha = texture(u_Alpha , m_tcoord).rgb;
	
	if(isFoliage == 1 && alpha.r <=0.06 ) // check if the tex value is less than a certain threshold then discard
		discard;
}