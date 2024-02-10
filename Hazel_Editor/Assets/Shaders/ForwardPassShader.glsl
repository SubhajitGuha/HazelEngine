#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 cord;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 Tangent;
layout (location = 4) in vec3 BiTangent;

out vec2 tcord;
out vec4 m_pos;
out vec4 m_curPos;
out vec4 m_oldPos;
out vec3 m_Normal;
out vec3 m_Tangent;
out vec3 m_BiTangent;

uniform mat4 u_ProjectionView;
uniform mat4 u_oldProjectionView;
uniform mat4 u_Model;
uniform mat4 u_View;

void main()
{
	vec4 clip_space = u_ProjectionView * u_Model * pos;
	m_curPos = clip_space;
	m_oldPos = u_oldProjectionView * u_Model *pos; //old-pos for creating the velocity buffer

	gl_Position = clip_space;
	tcord = cord;
	m_Normal = normalize(mat3(u_View * u_Model) * Normal);
	m_Tangent = normalize(mat3(u_View * u_Model) * Tangent);
	m_BiTangent = normalize(mat3(u_View * u_Model) * BiTangent);
	m_pos = u_View * u_Model * pos;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 gNormal;
layout (location = 1) out vec4 gVelocity;
layout (location = 2) out vec4 gColor;
layout (location = 3) out vec4 gRoughnessMetallic;

in vec4 m_pos;
in vec4 m_curPos; //current clip-space position
in vec4 m_oldPos; //previous clip-space position
in vec3 m_Normal;
in vec3 m_Tangent;
in vec3 m_BiTangent;
in vec2 tcord;

//PBR properties
uniform float Roughness;
uniform float Metallic;
uniform float Transperancy;
uniform vec4 m_color;


//Texture maps
uniform sampler2D u_Albedo;
uniform sampler2D u_Roughness;
uniform sampler2D u_NormalMap;

vec3 NormalMapping()
{
	vec3 normal = texture(u_NormalMap , tcord).rgb;
	normal = normal*2.0 - 1.0; //convert to -1 to 1
	mat3 TBN = mat3(m_Tangent , m_BiTangent , m_Normal);
	if(normal == vec3(1.0)) // if normal map is a White Texture then Lighting Calculation will be done by vertex Normal
		return m_Normal;
	else
		return normalize(TBN * normal);// to world space
}
vec3 GammaCorrection(in vec3 color)
{
	return pow(color, vec3(2.2)); //Gamma space
}

vec2 CalculateVelocity(in vec4 curPos,in vec4 oldPos)
{
	curPos /= curPos.w;
	curPos.xy = (curPos.xy+1.0) * 0.5;
	//curPos.y = 1.0 - curPos.y;

	oldPos /= oldPos.w;
	oldPos.xy = (oldPos.xy+1.0) * 0.5;
	//oldPos.y = 1.0 - oldPos.y;

	return (curPos - oldPos).xy;
}

void main()
{
	vec4 albedo = texture(u_Albedo, tcord) * m_color;
	mat4x4 thresholdMatrix = mat4x4(
	
		1.0 / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0,
		13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0,
		4.0 / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0,
		16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0
	);

	float val = thresholdMatrix[int(gl_FragCoord.x) % 4][int(gl_FragCoord.y) % 4];
	if(albedo.a < val)
		discard;	
	gNormal = vec4(NormalMapping(),1.0);
	gVelocity = vec4(CalculateVelocity(m_curPos, m_oldPos),0,1.0);
	gColor = vec4(GammaCorrection(albedo.rgb), 1.0);
	vec3 roughnessMetallic = texture(u_Roughness, tcord).xyz;
	gRoughnessMetallic = vec4(roughnessMetallic.r * Roughness,roughnessMetallic.g * Metallic,1,1);
}