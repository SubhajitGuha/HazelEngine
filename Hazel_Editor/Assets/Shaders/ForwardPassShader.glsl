#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 cord;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 Tangent;
layout (location = 4) in vec3 BiTangent;
layout (location = 5) in float slotindex;

out vec2 tcord;
out vec4 m_pos;
out vec3 m_Normal;
out vec3 m_Tangent;
out vec3 m_BiTangent;
flat out float m_slotindex;

uniform mat4 u_ProjectionView;
uniform mat4 u_Model;
uniform mat4 u_View;

void main()
{
	gl_Position = u_ProjectionView * u_Model * pos;
	m_slotindex = slotindex;
	tcord = cord;
	m_Normal = normalize(mat3(u_View * u_Model) * Normal);
	m_Tangent = normalize(mat3(u_View * u_Model) * Tangent);
	m_BiTangent = normalize(mat3(u_View * u_Model) * BiTangent);
	m_pos = u_View * u_Model * pos;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gColor;
layout (location = 3) out vec4 gRoughnessMetallic;


in vec4 m_pos;
in vec3 m_Normal;
in vec3 m_Tangent;
in vec3 m_BiTangent;
flat in float m_slotindex;
in vec2 tcord;

//PBR properties
uniform float Roughness;
uniform float Metallic;
uniform float Transperancy;
uniform vec4 m_color;


//Texture maps
uniform sampler2DArray u_Albedo;
uniform sampler2DArray u_Roughness;
uniform sampler2DArray u_NormalMap;

vec3 NormalMapping(int index) // index implies which material index normal map to use
{
	vec3 normal = texture(u_NormalMap , vec3(tcord,index)).rgb;
	normal = normal*2.0 - 1.0; //convert to -1 to 1
	mat3 TBN = mat3(m_Tangent , m_BiTangent , m_Normal);
	if(normal == vec3(1.0)) // if normal map is a White Texture then Lighting Calculation will be done by vertex Normal
		return m_Normal;
	else
		return normalize(TBN * normal);// to world space
}

void main()
{
	int index = int (m_slotindex);
	gPosition = vec4(m_pos.xyz,1.0);
	gNormal = vec4(NormalMapping(index),1.0);
	gColor = vec4(texture(u_Albedo,vec3(tcord,index)).rgb * m_color.rgb, 1.0);
	gRoughnessMetallic = vec4(texture(u_Roughness,vec3(tcord,index)).r*Roughness,Metallic,1,1);
}