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
	m_Normal = normalize(Normal);
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

#define MAX_LIGHTS 100
vec4 VertexPosition_LightSpace;

//shadow uniforms
uniform mat4 MatrixShadow[4];
uniform sampler2D ShadowMap[4];
uniform float Ranges[5];
uniform mat4 view;

uniform samplerCube env;
uniform sampler2D u_texture[32];
uniform sampler2DArray u_Albedo;
uniform sampler2DArray u_Roughness;
uniform float u_depth;

uniform vec3 EyePosition;

//Lights
uniform vec3 DirectionalLight_Direction; //sun light world position
uniform vec3 PointLight_Position[MAX_LIGHTS];
uniform vec3 PointLight_Color[MAX_LIGHTS];
uniform int Num_PointLights;

//PBR properties
uniform float Roughness;

vec3 PBR_Color = vec3(0.0);
vec3 radiance;

vec3 ks;
vec3 kd;
float vdoth;

float alpha = Roughness; //Roughness value
const float PI = 3.14159265359;

float metallic = 0.0;

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
	vec3 f0 = vec3(0.4); //take f0 as 0.04 fo non-metals and 0.4 for metals
	f0 = mix(f0,m_color.xyz,metallic);
	return f0 + (1.0 - f0) * pow(clamp(1.0 - VdotH, 0.0 ,1.0) , 5.0);
}

vec3 SpecularBRDF(vec3 LightDir,vec3 ViewDir)
{
	vec3 Half = normalize( ViewDir + LightDir);
	float NdotH = max(dot(m_Normal,Half) , 0.0);
	float NdotV = max(dot(m_Normal,ViewDir) , 0.0);
	float NdotL = max(dot(m_Normal,LightDir) , 0.0);
	float VdotH = max(dot(ViewDir,Half) , 0.0);

	vdoth = VdotH;

	float Dggx = NormalDistribution_GGX(NdotH);
	float Gggx = Geometry_GGX(NdotV) * Geometry_GGX(NdotL);
	vec3 fresnel = Fresnel(VdotH);

	float denominator = 4.0 * NdotL * NdotV + 0.0001;
	vec3 specular = (Dggx * Gggx * fresnel) / denominator;
	return specular;
}


int level = 3;
void main()
{
	int index = int (m_slotindex);

	alpha = texture(u_Roughness , vec3(tcord,index)).x * Roughness; //multiplying the texture-normal with the float val gives control on how much of the normal we need

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
	vec3 EyeDirection = normalize(EyePosition - m_pos.xyz/m_pos.w);


	//shadows
	vec3 p = VertexPosition_LightSpace.xyz/VertexPosition_LightSpace.w;
	p = p * 0.5 + 0.5;//convert -1 to +1 to 0 to 1 this is needed for getting the location in the texture
	float bias = 0.00001;//bias to resolve the artifact
	float shadow = texture(ShadowMap[level],p.xy).r  < p.z - bias? 0:1;// sample the depth map and check the p.xy coordinate of depth map with the p.z value
	
	//environment reflections
	vec3 Light_dir_i = reflect(-EyeDirection,m_Normal);
	vec3 EnvironmentCol = texture(env,Light_dir_i).xyz * (1.0 - alpha) ;

	//ambiance
		vec3 ambiant = m_color.xyz * vec3(0.1,0.1,0.1);

		vdoth = max(dot( EyeDirection, normalize( EyeDirection + DirectionalLight_Direction)) ,0.0);//for directional light
		ks = Fresnel(vdoth);
		kd = vec3(1.0) - ks;
		kd *= (1.0 - metallic);

	PBR_Color += ( (kd * texture(u_Albedo, vec3(tcord , index)).xyz * m_color.xyz / PI) + SpecularBRDF(DirectionalLight_Direction , EyeDirection) ) * shadow * max(dot(m_Normal,DirectionalLight_Direction), 0.0) ; //for directional light (no attenuation)

	//color=vec4(PointLight_Position[0],1.0);
	for(int i=0 ; i< Num_PointLights ; i++)
	{
		vec3 LightDirection = normalize(PointLight_Position[i] - m_pos.xyz/m_pos.w); //for point light

		//specular
		vec3 specular = SpecularBRDF(LightDirection , EyeDirection) ;
		ks = Fresnel(vdoth);

		//diffuse
		kd = vec3(1.0) - ks;
		kd *= (1.0 - metallic);
		vec3 diffuse = kd * texture(u_Albedo,vec3(tcord , index)).xyz * m_color.xyz / PI; // no alpha channel is being used

		float dist = length(PointLight_Position[i] - m_pos.xyz/m_pos.w);
		float attenuation = 1 / ( 0.01 * dist * dist ); //attenuation is for point and spot light
		radiance = PointLight_Color[i] * attenuation;
		
		float NdotL = max(dot(m_Normal,LightDirection), 0.0);
		PBR_Color += (diffuse + specular)  * radiance * NdotL ; //for Point light (attenuation)
	}

	PBR_Color += ambiant;
	//PBR_Color = PBR_Color / (PBR_Color + vec3(1.0));
	//PBR_Color = pow(PBR_Color, vec3(1.0/2.2)); 

	color = vec4(PBR_Color,1);
	//color = texture(u_Roughness, vec3(tcord.x,tcord.y , 1.0));
}