#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 cord;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 Tangent;
layout (location = 4) in vec3 BiTangent;
layout (location = 5) in float materialindex;
layout (location = 6) in mat4 instance_mm;

out vec2 tcord;
out vec4 m_pos;
out vec3 m_Normal;
out vec3 m_Tangent;
out vec3 m_BiTangent;
out vec3 m_VertexColor;
flat out float m_materialindex;

uniform mat4 u_ProjectionView;
uniform mat4 u_Projection;
uniform mat4 u_Model;
uniform float u_Time;
uniform vec4 m_color;
uniform sampler2D Noise;
float amplitude=40;
float wsAmplitude=0.6;

mat4 CreateRotationMat(float x, float y, float z)
{
	x= radians(x);
	y= radians(y);
	z= radians(z);

	mat4 aroundX = mat4(
	1,0,0,0,
	0,cos(x),sin(x),0,
	0,-sin(x),cos(x),0,
	0,0,0,1);

	mat4 aroundY = mat4(
	cos(y),0,-sin(y),0,
	0,1,0,0,
	sin(y),0,cos(y),0,
	0,0,0,1);

	mat4 aroundZ = mat4(
	cos(z),sin(z),0,0,
	-sin(z),cos(z),0,0,
	0,0,1,0,
	0,0,0,1);

	return aroundX * aroundY * aroundZ;
}

mat4 CreateScaleMatrix(float scale)
{
	return mat4(
	scale,0,0,0,
	0,scale,0,0,
	0,0,scale,0,
	0,0,0,1
	);
}
void main()
{
	mat4 wsGrass = u_Model * instance_mm;
	vec4 wsVertexPos = wsGrass * pos;
	vec3 origin = vec3(wsGrass[3][0],wsGrass[3][1],wsGrass[3][2]);
	float factor = distance(wsVertexPos.xyz , origin)/10;	
	if(factor<0)
		factor = 0;
	
	vec3 coord = mod(abs(wsVertexPos.xyz),256);
	coord/=vec3(256);

	vec3 noise = texture(Noise , coord.xz*2 ).rgb*2;
	vec3 rotVal = vec3(sin(u_Time + noise.r)*0.5+0.5+1) *factor*amplitude;
	vec3 ws_rotVal = vec3(noise.r*sin(u_Time)*0.5+0.5) *factor*wsAmplitude;

	mat4 ws_rot = CreateRotationMat(0, 0, ws_rotVal.y);
	mat4 rot = CreateRotationMat(0, rotVal.y, 0);

	float val = texture(Noise,coord.xz*10).r;

	gl_Position = u_ProjectionView * ws_rot * wsGrass * rot * CreateScaleMatrix(val*2) * pos;

	m_VertexColor = factor * m_color.xyz;
	m_materialindex = materialindex;
	tcord = cord;
	m_Normal = normalize(mat3(wsGrass ) * Normal );
	m_Tangent = normalize(mat3(wsGrass ) * Tangent );
	m_BiTangent = normalize(mat3(wsGrass ) * BiTangent );
	m_pos = ws_rot * wsGrass * rot * CreateScaleMatrix(val*2) * pos;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

//pbr mapping
//the u_Roughness texture contains "opacity map" on R-channel "Roughness map" on G-channel and "Ambient occlusion" on B-channel

in vec4 m_pos;
in vec3 m_Normal;
in vec3 m_Tangent;
in vec3 m_BiTangent;
in vec3 m_VertexColor;
flat in float m_materialindex;
in vec2 tcord;

#define MAX_LIGHTS 100
vec4 VertexPosition_LightSpace;

//shadow uniforms
uniform mat4 MatrixShadow[4];
uniform sampler2D ShadowMap[4];
uniform float Ranges[5];
uniform mat4 view;

uniform samplerCube diffuse_env;
uniform samplerCube specular_env;

uniform sampler2D SSAO;
uniform sampler2DArray u_Albedo;
uniform sampler2DArray u_Roughness;
uniform sampler2DArray u_NormalMap;
uniform float u_depth;
uniform mat4 u_ProjectionView;
uniform mat4 u_Projection;

uniform vec3 EyePosition;
uniform vec4 m_color;

//Lights
uniform vec3 DirectionalLight_Direction; //sun light world position
uniform vec3 PointLight_Position[MAX_LIGHTS];
uniform vec3 PointLight_Color[MAX_LIGHTS];
uniform int Num_PointLights;
uniform float SunLight_Intensity;
uniform vec3 SunLight_Color;

//PBR properties
uniform float Roughness;
uniform float Metallic;

vec3 PBR_Color = vec3(0.0);
vec3 radiance;

vec3 ks;
vec3 kd;
float vdoth;

float alpha = Roughness; //Roughness value
const float PI = 3.14159265359;
#define MAX_MIP_LEVEL 28
int level = 3; // cascade levels
float NdotL = 1.0;

//Noise
uniform sampler2D Noise;
//Colors for variation
vec4 color1 = vec4(0.6,0.8,0.1,1.0);
vec4 color2 = vec4(0.9,0.4,0.1,1.0);
vec4 color3 = vec4(0.1,0.8,0.6,1.0);
vec4 color4 = vec4(0.6,0.8,0.1,1.0);

vec3 NormalMapping(int index) // index implies which material index normal map to use
{
	vec3 normal = texture(u_NormalMap , vec3(tcord,index)).rgb;
	normal = normal*2.0 - 1.0;
	mat3 TBN = mat3(m_Tangent , m_BiTangent , m_Normal);
	if(normal == vec3(1.0))
		return m_Normal;
	else
		return normalize(TBN * normal);
}

float CalculateShadow(int cascade_level)
{
	float ShadowSum = 0.0;
	vec3 p = VertexPosition_LightSpace.xyz/VertexPosition_LightSpace.w;
	p = p * 0.5 + 0.5;//convert -1 to +1 to 0 to 1 this is needed for getting the location in the texture
	float bias = 0.0005*tan(acos(NdotL));//bias to resolve the artifact
	float TexelSize = 1.0/textureSize(ShadowMap[cascade_level],0).x; // 4k texture

	for(int i=-1; i <=1; i++)
	{
		for(int j=-1; j<=1; j++)
		{
			vec2 offset = vec2(i,j) * TexelSize; 
			float depth = texture(ShadowMap[cascade_level],p.xy + offset).r;
			if(depth + bias > p.z)
				ShadowSum+=1;
		}
	}
	//depth < p.z - bias? 0:1;// sample the depth map and check the p.xy coordinate of depth map with the p.z value
	
	return ShadowSum/9.0;
}

float NormalDistribution_GGX(float NdotH)
{
	float alpha2 =  pow(alpha,4); // alpha is actually the Roughness
	return alpha2 / (PI * pow( (pow(NdotH,2) * (alpha2 - 1.0) + 1.0) ,2) ) ;
}

float Geometry_GGX(float dp) //dp = Dot Product
{
	float k = pow(alpha+1,2) / 8.0;
	return dp/(dp * (1-k) + k);
}

vec3 Fresnel(float VdotH)
{
	vec3 f0;

	if(Metallic == 0.0)
		f0 = vec3(0.04); // for non metallic
	else
		f0 = vec3(0.4); // for metallic
	f0 = mix(f0,m_color.xyz,Metallic);
	//return f0 + (1.0 - f0) * pow(clamp(1.0 - VdotH, 0.0 ,1.0) , 5.0);
	//greater roughness = lesser fresnel value
	return f0 + (max(vec3(1.0- alpha),f0) - f0) * pow(clamp(1.0 - VdotH, 0.0 ,1.0) , 5.0);
}

vec3 SpecularBRDF(vec3 LightDir,vec3 ViewDir , vec3 Normal)
{
	vec3 Half = normalize( ViewDir + LightDir);
	float NdotH = max(dot(Normal,Half) , 0.0);
	float NdotV = max(dot(Normal,ViewDir) , 0.000001);
	float NdotL = max(dot(Normal,LightDir) , 0.000001);
	float VdotH = max(dot(ViewDir,Half) , 0.0);

	vdoth = VdotH;

	float Dggx = NormalDistribution_GGX(NdotH);
	float Gggx = Geometry_GGX(NdotV) * Geometry_GGX(NdotL);
	vec3 fresnel = Fresnel(VdotH);

	float denominator = 4.0 * NdotL * NdotV + 0.0001;
	vec3 specular = (Dggx * Gggx * fresnel) / denominator;
	return specular;
}

vec3 ColorCorrection(vec3 color)
{
	color = clamp(color,0,1);
	color = pow(color, vec3(1.0/2.2)); //Gamma correction

	color = clamp(color,0,1);
	color = vec3(1.0) - exp(-color * 3);//exposure

	//color = clamp(color,0,1);
	//color = mix(vec3(dot(color,vec3(0.299,0.587,0.114))), color,2);//saturation

	color = clamp(color,0,1);
	color = 1.6*(color-0.5) + 0.5 +0.02 ; //contrast

	return color;
}

float ao = 1.0;
void main()
{
	int index = int (m_materialindex);
	
	float Transparency =  texture(u_Albedo, vec3(tcord , index)).a;
	//float opacity = texture(u_Roughness , vec3(tcord,index)).r;// Opacity on R-Channel
	//color = vec4(vec3(opacity),1.0);
	//if(opacity <= 0.1)
		//discard; // if the texture value is less than a certain threshold then discard that pixel

	vec3 Modified_Normal = normalize(m_Normal);

	//alpha = texture(u_Roughness , vec3(tcord,index)).g * Roughness; //multiplying the texture-Roughness with the float val gives control on how much of the Roughness we need
	alpha = Roughness;
	//ao = texture(u_Roughness , vec3(tcord,index)).b; // Ambient occlusion on B-Channel

	vec4 vert_pos = view * m_pos; //get depth value(z value) in the camera-view space
	vec3 v_position = vert_pos.xyz/vert_pos.w;
	float depth = abs(v_position.z);

	for(int i=0;i<4;i++)
	{
		if(depth<Ranges[i])
		{
			level = i;
			break;
		}
	}

	VertexPosition_LightSpace = MatrixShadow[level] * m_pos;

	vec3 DirectionalLight_Direction = normalize(-DirectionalLight_Direction );//for directional light as it has no concept of position
	NdotL = max(dot(Modified_Normal,DirectionalLight_Direction),0.0001);
	vec3 EyeDirection = normalize(EyePosition - m_pos.xyz/m_pos.w);

	//shadows
	float shadow = CalculateShadow(level);

	//diffuse_environment reflections
	vec3 Light_dir_i = reflect(-EyeDirection,Modified_Normal);
	//vec3 diffuse_environmentCol = texture(diffuse_env,Light_dir_i).xyz * (1.0 - alpha) ;

	vdoth = max(dot( EyeDirection, normalize( EyeDirection + DirectionalLight_Direction)) ,0.0);//for directional light
	ks = Fresnel(vdoth);
	kd = vec3(1.0) - ks;
	kd *= (1.0 - Metallic);

	vec3 IBL_diffuse =  texture(diffuse_env,Modified_Normal).rgb * kd; //sampling the irradiance map
	vec3 BRDFintegration =  ks*alpha + max(dot(Modified_Normal,DirectionalLight_Direction),0.001) ;// we preapare the multiplication factor by the roughness and the NdotL value
	vec3 IBL_specular = textureLod(specular_env,Light_dir_i , MAX_MIP_LEVEL * alpha).rgb * BRDFintegration ; //sample the the environment map at varying mip level
	
	vec4 coordinate = u_ProjectionView * m_pos;
	coordinate.xyz /= coordinate.w;
	coordinate.xyz = coordinate.xyz*0.5 + 0.5;
	//ambiance
		vec3 ambiant = (IBL_diffuse + IBL_specular) * texture(u_Albedo, vec3(tcord , index)).xyz * m_color.xyz * pow(texture(SSAO,coordinate.xy).r,1);

	vec3 coord = mod(abs(m_pos.xyz),256);
	coord/=vec3(256);

	//vec3 col_factor =  texture(Noise,coord.xz).rgb;
	//vec4 grass_col = mix(color1,color2 , col_factor.r);
	//grass_col = mix(grass_col,color3, col_factor.g);
	//grass_col = mix(grass_col,color4, col_factor.b);

	PBR_Color += ( ((kd * texture(u_Albedo, vec3(tcord , index)).xyz * (m_VertexColor.xyz)) / PI) + SpecularBRDF(DirectionalLight_Direction , EyeDirection , Modified_Normal) ) * (shadow * SunLight_Color * SunLight_Intensity) * max(dot(Modified_Normal,DirectionalLight_Direction), 0.0) ; //for directional light (no attenuation)


	PBR_Color += ambiant;
	PBR_Color = clamp(ColorCorrection(PBR_Color),0.0,1.0);
	
	color = vec4(PBR_Color.xyz,Transparency);
}