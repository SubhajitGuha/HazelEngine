#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 cord;
layout (location = 2) in vec4 color;
layout (location = 3) in vec3 Normal;
layout (location = 4) in float slotindex;

out vec2 tcord;
out vec4 m_pos;
out vec4 m_color;
out vec3 m_Normal;
flat out float m_slotindex;

uniform mat4 u_ProjectionView;
uniform mat4 u_ModelTransform;

void main()
{
	gl_Position = u_ProjectionView * pos;
	m_color = color;
	m_slotindex = slotindex;
	tcord = cord;
	m_Normal = Normal;
	m_pos = pos;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

in vec4 m_color;
in vec4 m_pos;
in vec3 m_Normal;
flat in float m_slotindex;
in vec2 tcord;

vec4 VertexPosition_LightSpace;

//shadow uniforms
uniform mat4 MatrixShadow[4];
uniform sampler2D ShadowMap[4];
uniform float Ranges[5];
uniform mat4 view;

uniform samplerCube env;
uniform sampler2D u_Texture[32];
uniform vec4 u_color;
uniform vec3 LightPosition;//world position
uniform vec3 EyePosition;

int level = 3;
void main()
{
	int index = int (m_slotindex);

	vec4 vert_pos = view * m_pos;
	vec3 v_position = vert_pos.xyz/vert_pos.w;
	float depth = abs(v_position.z);

	for(int i=0;i<4;i++)
	{
		if(depth<Ranges[i])
		{
			//color = vec4(0,1,i/4.0,1);
			level = i;
			break;
		}
	}

	VertexPosition_LightSpace = MatrixShadow[level] * m_pos;

	//color = vec4(m_pos.xyz,1.0);
	vec3 LightDirection = normalize(LightPosition - vec3(m_pos.x,m_pos.y,m_pos.z));

	//shadows
	vec3 p = VertexPosition_LightSpace.xyz/VertexPosition_LightSpace.w;
	p = p * 0.5 + 0.5;
	float bias = 0.00002;//bias to resolve the artifact
	float shadow = texture(ShadowMap[level],p.xy).r  < p.z - bias? 0:1;// sample the depth map and check the p.xy coordinate of depth map with the p.z value
	
	//diffuse
	float brightness = dot(LightDirection , m_Normal);
	vec4 diffuse = m_color * vec4(brightness,brightness,brightness,1) * vec4(shadow,shadow,shadow,1);

	//ambiance
	vec4 ambiant = m_color * vec4(0.2,0.2,0.2,1.0);

	//specular
	vec3 EyeDirection = normalize(EyePosition - vec3(m_pos));
	vec3 Reflected_Light = reflect(-LightDirection , m_Normal);
	float s_brightness = clamp(dot(EyeDirection , Reflected_Light),0,1);
	float b = pow(s_brightness,100);//to reduce the size of specular see the graph of (cosx)^2
	vec4 specular = m_color * vec4(b,b,b,1) * vec4(shadow,shadow,shadow,1);

	//environment reflections
	vec3 Light_dir_i = reflect(-EyeDirection,m_Normal);
	vec4 EnvironmentCol = m_color * texture(env,Light_dir_i) ;
	//vec4 CubeMap = m_color * texture(cube_map,Light_dir_i);


	float dist = length(LightPosition-vec3(m_pos));
	float attenuation = 1/(0.1 + 0*dist + 0.008 * dist*dist);
	color= texture(u_Texture[index],tcord) * clamp(diffuse,0,attenuation) + ambiant + clamp(specular,0,attenuation) ;

}