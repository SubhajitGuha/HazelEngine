#shader vertex
#version 410 core
layout (location = 0) in vec4 position;
layout (location = 1) in vec4 cord;

out vec2 tcord;

//jitter for temporal anti-aliasing
uniform float jitterX;
uniform float jitterY;

void main()
{
	vec4 clip_space = position;
	clip_space += vec4(jitterX*clip_space.w, jitterY*clip_space.w, 0.0, 0.0);
	gl_Position = clip_space;
	tcord = cord.xy;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

in vec2 tcord;

uniform sampler2D depthBuffer;
uniform sampler2D gNormal;
uniform sampler2D gColor;
uniform sampler2D gRoughnessMetallic;
uniform sampler2D gVelocity;

#define MAX_LIGHTS 100
vec4 VertexPosition_LightSpace;

//shadow uniforms
uniform mat4 MatrixShadow[4];
uniform sampler2D ShadowMap[4];
uniform float Ranges[5];
uniform mat4 u_View;
uniform mat4 u_Projection;

//IBL
uniform samplerCube diffuse_env;
uniform samplerCube specular_env;
uniform sampler2D BRDF_LUT;

uniform sampler2D SSAO;
uniform vec3 EyePosition;

//Lights
uniform vec3 DirectionalLight_Direction; //sun light world position
uniform vec3 SunLight_Color;
uniform float SunLight_Intensity;
uniform vec3 PointLight_Position[MAX_LIGHTS];
uniform vec3 PointLight_Color[MAX_LIGHTS];
uniform int Num_PointLights;

//TAA history buffer
uniform sampler2D History_Buffer;

vec3 PBR_Color = vec3(0.0);
vec3 radiance;

vec3 ks;
vec3 kd;
vec3 F0;

float alpha = 0; //Roughness value
float Metallic = 0;
const float PI = 3.14159265359;
#define MAX_MIP_LEVEL 4

int level = 3; // cascade levels

float CalculateShadow(int cascade_level)
{
	float ShadowSum = 0.0;
	vec3 p = VertexPosition_LightSpace.xyz/VertexPosition_LightSpace.w;
	p = p * 0.5 + 0.5;//convert -1 to +1 to 0 to 1 this is needed for getting the location in the texture
	float bias = 0.0001;//bias to resolve the artifact
	float TexelSize = 1.0/textureSize(ShadowMap[cascade_level],0).x;

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
	float totalShadow = ShadowSum/9.0;
	//totalShadow = pow(totalShadow,16.0);
	return totalShadow;
}

float NormalDistribution_GGX(float NdotH)
{
	float alpha2 =  pow(alpha,4); // alpha is actually the Roughness
	return alpha2 / (PI * pow( (pow(NdotH,2) * (alpha2 - 1.0) + 1.0) ,2) ) ;// making the power value = 4 as higher = greater lobe
}

float Geometry_GGX(float dp) //dp = Dot Product
{
	float k = pow(alpha+1,2) / 8.0;
	return dp/(dp * (1-k) + k);
}

vec3 Fresnel(float VdotH)
{	
	return F0 + (1.0 - F0) * pow(clamp(1.0 - VdotH, 0.0 ,1.0) , 5.0);
}

vec3 FresnelSchlickRoughness(float VdotH, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0);
}

vec3 SpecularBRDF(vec3 LightDir,vec3 ViewDir, vec3 Normal)
{
	vec3 Half = normalize( ViewDir + LightDir);
	float NdotH = max(dot(Normal,Half) , 0.0);
	float NdotV = max(dot(Normal,ViewDir) , 0.000001);
	float NdotL = max(dot(Normal,LightDir) , 0.000001);
	float VdotH = max(dot(ViewDir,Half) , 0.0);

	float Dggx = NormalDistribution_GGX(NdotH);
	float Gggx = Geometry_GGX(NdotV) * Geometry_GGX(NdotL);
	vec3 fresnel = Fresnel(VdotH);

	float denominator = 4.0 * NdotL * NdotV + 0.0001;
	vec3 specular = (Dggx * Gggx * fresnel) / denominator;
	return specular;
}

vec3 ColorCorrection(vec3 color)
{
	//color = clamp(color,0,1);
	//color = pow(color, vec3(1.0/2.2)); //Gamma space

	//color = clamp(color,0,1);
	color = vec3(1.0) - exp(-color * 1.1);//exposure

	//color = clamp(color,0,1);
	color = mix(vec3(dot(color,vec3(0.299,0.587,0.114))), color,1.0);//saturation

	//color = clamp(color,0,1);
	color = 1.03*(color-0.5) + 0.5 + 0.00 ; //contrast

	return color;
}

vec3 ACESFilm(vec3 x)
{
	float a = 2.51;
	float b = 0.03;
	float c = 2.43;
	float d = 0.59;
	float e = 0.14;
	return clamp((x*(a*x+b))/(x*(c*x+d)+e),0.0,1.0);
}

void main()
{
	float z = texture(depthBuffer,tcord).r; //in range 0-1
	if(z == 1.0 ) //to blend in the sky
       discard;

	vec4 clip_space = vec4(tcord*2.0-1.0,z*2.0-1.0,1.0); //construct clip-space by converting tex-coord and depth values form 0-1 to -1-1
	vec4 view_space = inverse(u_Projection) * clip_space;
	view_space /= view_space.w; //perspective divide
	vec4 world_space = inverse(u_View)*view_space; //recreate world-space pos from depthBuffer

	vec3 Modified_Normal = normalize(mat3(inverse(u_View)) * texture(gNormal,tcord).xyz); //in ws
	vec4 m_pos = world_space;//in ws
	m_pos = vec4(m_pos.xyz, 1.0);

	vec4 m_Color = texture(gColor , tcord);
	vec4 RoughnessMetallic = texture(gRoughnessMetallic , tcord);

	alpha = RoughnessMetallic.r;
	Metallic = RoughnessMetallic.g;

	vec3 v_position = view_space.xyz;
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
	vec3 EyeDirection = normalize( EyePosition - m_pos.xyz);

	//shadows
	float shadow = CalculateShadow(level);

	//diffuse_environment reflections
	vec3 Light_dir_i = reflect(-EyeDirection,Modified_Normal);

	//F0 is 0.04 for non-metallic and albedo for metallics 
	F0 = vec3(0.04);
	F0 = mix(F0, m_Color.xyz, Metallic);
	
//------------------------------------ambient-----------------------------------------------------------------
	float ndotv = max(dot( EyeDirection, Modified_Normal),0.0);//for directional light
	
	ks = FresnelSchlickRoughness(ndotv, alpha);
	kd = vec3(1.0) - ks;
	kd *= (1.0 - Metallic);

	vec3 IBL_diffuse =  texture(diffuse_env,Modified_Normal).rgb * kd; //sampling the irradiance map
	vec2 BRDFintegration =  texture(BRDF_LUT, vec2(max(dot(Modified_Normal , EyeDirection), 0.0), alpha)).rg;// we preapare the multiplication factor by the roughness and the NdotL value
	vec3 IBL_specular = textureLod(specular_env, Light_dir_i , MAX_MIP_LEVEL * alpha).rgb * (ks * BRDFintegration.x + BRDFintegration.y); //sample the the environment map at varying mip level
	
	//ambiance
		vec3 ambiant = (IBL_diffuse * m_Color.xyz + IBL_specular) * pow(texture(SSAO,tcord).r,1.0);

//----------------------------------Sun Light-------------------------------------------------------------------
	float vdoth = max(dot(EyeDirection, normalize(DirectionalLight_Direction + EyeDirection)),0.0);
	ks = Fresnel(vdoth);
	kd = vec3(1.0) - ks;
	kd *= (1.0 - Metallic);
	PBR_Color += ( (kd * m_Color.xyz / PI) + SpecularBRDF(DirectionalLight_Direction , EyeDirection , Modified_Normal) ) * (shadow * SunLight_Color * SunLight_Intensity) * max(dot(Modified_Normal,DirectionalLight_Direction), 0.001) ; //for directional light (no attenuation)

//--------------------------------Point Lights------------------------------------------------------------------
	for(int i=0 ; i< Num_PointLights ; i++)
	{
		vec3 LightDirection = normalize(PointLight_Position[i] - m_pos.xyz/m_pos.w); //for point light
		vec3 H = normalize(LightDirection + EyeDirection); //Half vector
		//specular
		vec3 specular = SpecularBRDF(LightDirection , EyeDirection ,Modified_Normal) ;
		ks = Fresnel(max(dot(H,EyeDirection),0.0)); //VdotH

		//diffuse
		kd = vec3(1.0) - ks;
		kd *= (1.0 - Metallic);
		vec3 diffuse = kd * m_Color.xyz / PI; // no alpha channel is being used

		float dist = length(PointLight_Position[i] - m_pos.xyz/m_pos.w);
		float attenuation = 1 / ( 0.01 * dist * dist ); //attenuation is for point and spot light
		radiance = PointLight_Color[i] * attenuation;
		
		float NdotL = max(dot(Modified_Normal,LightDirection), 0.1);
		PBR_Color += (diffuse + specular)  * radiance * NdotL ; //for Point light (attenuation)
	}

	PBR_Color += ambiant;

	//color = vec4(ACESFilm(PBR_Color),1.0);
	color = vec4((PBR_Color),1.0);

}