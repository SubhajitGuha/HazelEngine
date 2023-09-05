#shader vertex
#version 410 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 cord;

out vec2 TexCoord;

void main()
{
	gl_Position = vec4(pos,1.0);
	TexCoord = cord;
}




#shader tessellation control
#version 410 core
layout (vertices = 4) out;
const float MAX_TESS_LEVEL = 64;
const float MIN_TESS_LEVEL = 4;
const float MAX_CAM_DIST = 1000;
const float MIN_CAM_DIST = 0;

in vec2 TexCoord[];
out TCS_Data
{
	vec2 TexCoord_TCS;
} tcs_data[];

uniform vec3 camPos; //in ws
uniform mat4 u_Model;
uniform mat4 u_View;

void main()
{
	if(gl_InvocationID == 0)
	{
		vec4 p1 = u_View * u_Model * gl_in[0].gl_Position;
		vec4 p2 = u_View * u_Model * gl_in[1].gl_Position;
		vec4 p3 = u_View * u_Model * gl_in[2].gl_Position;
		vec4 p4 = u_View * u_Model * gl_in[3].gl_Position;
		
		float dist01 = clamp( (abs(p1.z) - MIN_CAM_DIST) / (MAX_CAM_DIST-MIN_CAM_DIST), 0.0, 1.0);
		float dist02 = clamp( (abs(p2.z) - MIN_CAM_DIST) / (MAX_CAM_DIST-MIN_CAM_DIST), 0.0, 1.0);
		float dist03 = clamp( (abs(p3.z) - MIN_CAM_DIST) / (MAX_CAM_DIST-MIN_CAM_DIST), 0.0, 1.0);
		float dist04 = clamp( (abs(p4.z) - MIN_CAM_DIST) / (MAX_CAM_DIST-MIN_CAM_DIST), 0.0, 1.0);

		float TessValue01 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist01,dist03));
		float TessValue02 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist01,dist02));
		float TessValue03 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist02,dist04));
		float TessValue04 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(dist04,dist03));

		gl_TessLevelOuter[0] = TessValue01;
		gl_TessLevelOuter[1] = TessValue02;
		gl_TessLevelOuter[2] = TessValue03;
		gl_TessLevelOuter[3] = TessValue04;
							   
		gl_TessLevelInner[0] = max(TessValue02,TessValue04);
		gl_TessLevelInner[1] = max(TessValue01,TessValue03);
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	tcs_data[gl_InvocationID].TexCoord_TCS = TexCoord[gl_InvocationID];
}



#shader tessellation evaluation
#version 410 core
layout (quads,fractional_even_spacing ,ccw) in;

in TCS_Data
{
	vec2 TexCoord_TCS;
} tcs_data[];

out GS_Data
{
	vec2 TexCoord;
}gs_data;

uniform sampler2D u_HeightMap;
uniform mat4 u_ProjectionView;
uniform mat4 u_Model;
uniform float HEIGHT_SCALE;

vec2 texture_size;

vec4 Interpolate(vec4 v0, vec4 v1, vec4 v2, vec4 v3)
{
	vec4 a = mix(v0,v1,gl_TessCoord.x);
	vec4 b = mix(v2,v3,gl_TessCoord.x);
	return mix(a,b,gl_TessCoord.y);
}

vec2 Interpolate(vec2 v0, vec2 v1, vec2 v2, vec2 v3)
{
	vec2 a = mix(v0,v1,gl_TessCoord.x);
	vec2 b = mix(v2,v3,gl_TessCoord.x);
	return mix(a,b,gl_TessCoord.y);
}

void main()
{
	//in object space
	vec4 oldPos = Interpolate(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position, gl_in[3].gl_Position);
	vec2 texCoord = Interpolate(tcs_data[0].TexCoord_TCS, tcs_data[1].TexCoord_TCS, tcs_data[2].TexCoord_TCS, tcs_data[3].TexCoord_TCS);
	gs_data.TexCoord = texCoord;
	float Height = texture(u_HeightMap,texCoord).r * HEIGHT_SCALE;
	vec4 newPos = vec4(0,Height,0,0); //as proj_view and model matrix is same for all vertex
	
	gl_Position = oldPos + newPos;
}


#shader geometry
#version 410 core
layout (triangles) in;
layout (triangle_strip,max_vertices = 3) out;

uniform mat4 u_ProjectionView;
uniform mat4 u_Model;
uniform float HEIGHT_SCALE;
uniform float HillLevel;
uniform sampler2D u_perlinNoise;
uniform float FoliageHeight;
uniform float Time;

in GS_Data
{
	vec2 TexCoord;
}gs_data[];

out FS_Data
{
	vec2 TexCoord;
}fs_data;

out Grass
{
	vec4 Pos;
	vec3 Normal;
}grass_data;


uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

// Pseudo-random value in half-open range [0:1].
float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }

void main()
{
	vec4 VertexPos0_ws = u_Model * gl_in[0].gl_Position;
	vec4 VertexPos1_ws = u_Model * gl_in[1].gl_Position;
	vec4 VertexPos2_ws = u_Model * gl_in[2].gl_Position;


	gl_Position = u_ProjectionView * VertexPos0_ws;
	fs_data.TexCoord = gs_data[0].TexCoord;
	grass_data.Pos = VertexPos0_ws;
	EmitVertex();

	gl_Position = u_ProjectionView * VertexPos1_ws;
	fs_data.TexCoord = gs_data[1].TexCoord;
	grass_data.Pos = VertexPos1_ws;
	EmitVertex();

	gl_Position = u_ProjectionView * VertexPos2_ws;
	fs_data.TexCoord = gs_data[2].TexCoord;
	grass_data.Pos = VertexPos2_ws;
	EmitVertex();
	
}



#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

in FS_Data
{
	vec2 TexCoord;
}fs_data;

in Grass
{
	vec4 Pos;
	vec3 Normal;
}grass_data;

//shadow uniforms
uniform mat4 MatrixShadow[4];
uniform sampler2D ShadowMap[4];
uniform float Ranges[5];
uniform mat4 view;
uniform mat4 u_ProjectionView;

uniform float WaterLevel;
uniform float HillLevel;
uniform float MountainLevel;
uniform float u_Intensity;
uniform float HEIGHT_SCALE = 1000;
uniform float u_Tiling;
uniform float SunLight_Intensity;

uniform vec3 u_CameraPos;
uniform vec3 DirectionalLight_Direction;
uniform vec3 SunLight_Color;

uniform sampler2D SSAO;
uniform sampler2D u_HeightMap;
uniform sampler2D u_Albedo;
uniform sampler2D u_Roughness;
uniform sampler2D u_Normal;
uniform samplerCube diffuse_env;
uniform samplerCube specular_env;

vec4 VertexPosition_LightSpace;
vec3 PBR_Color = vec3(0.0);
vec3 radiance;
float vdoth;
vec3 ks;
vec3 kd;
float alpha = 1.0;
const float PI = 3.14159265359;
float Metallic = 0.0;
#define MAX_MIP_LEVEL 28
int level = 3; // cascade levels
float NdotL = 1.0;

vec3 CalculateNormal(vec2 texCoord , vec2 texelSize)
{
	float left = texture(u_HeightMap,texCoord + vec2(-texelSize.x,0.0)).r * HEIGHT_SCALE  *2.0 - HEIGHT_SCALE;
	float right = texture(u_HeightMap,texCoord + vec2(texelSize.x,0.0)).r * HEIGHT_SCALE *2.0 - HEIGHT_SCALE;
	float up = texture(u_HeightMap,texCoord + vec2(0.0 , texelSize.y)).r * HEIGHT_SCALE *2.0 - HEIGHT_SCALE;
	float down = texture(u_HeightMap,texCoord + vec2(0.0 , -texelSize.y)).r * HEIGHT_SCALE *2.0 - HEIGHT_SCALE;

	return normalize(vec3(down-up,2.0,left-right));
}

vec3 CalculateTangent(vec2 texCoord , vec2 texelSize)
{
	float left = texture(u_HeightMap,texCoord + vec2(-texelSize.x,0.0)).r * HEIGHT_SCALE  *2.0 - HEIGHT_SCALE;
	float right = texture(u_HeightMap,texCoord + vec2(texelSize.x,0.0)).r * HEIGHT_SCALE *2.0 - HEIGHT_SCALE;
	float up = texture(u_HeightMap,texCoord + vec2(0.0 , texelSize.y)).r * HEIGHT_SCALE *2.0 - HEIGHT_SCALE;
	float down = texture(u_HeightMap,texCoord + vec2(0.0 , -texelSize.y)).r * HEIGHT_SCALE *2.0 - HEIGHT_SCALE;

	return normalize(vec3(2.0,0.0,left-right));
}

mat3 TBN(vec2 texCoord , vec2 texelSize)
{
	vec3 N = CalculateNormal(texCoord , texelSize);
	vec3 T = CalculateTangent(texCoord , texelSize);
	vec3 B = cross(T,N);

	return mat3(T,B,N);
}

float CalculateShadow(int cascade_level)
{
	float ShadowSum = 0.0;
	vec3 p = VertexPosition_LightSpace.xyz/VertexPosition_LightSpace.w;
	p = p * 0.5 + 0.5;//convert -1 to +1 to 0 to 1 this is needed for getting the location in the texture
	float bias = 0.00001*tan(acos(NdotL));//bias to resolve the artifact
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
	//depth < p.z - bias? 0:1;// sample the depth map and check the p.xy coordinate of depth map with the p.z value
	
	return ShadowSum/9.0;
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
	vec3 f0;

	if(Metallic == 0.0)
		f0 = vec3(0.04); // for non metallic
	else
		f0 = vec3(0.4); // for metallic
	//return f0 + (1.0 - f0) * pow(clamp(1.0 - VdotH, 0.0 ,1.0) , 5.0);
	//greater roughness = lesser fresnel value
	return f0 + (max(vec3(1.0- alpha),f0) - f0) * pow(clamp(1.0 - VdotH, 0.0 ,1.0) , 5.0);
}

vec3 SpecularBRDF(vec3 LightDir,vec3 ViewDir, vec3 Normal)
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

	color = clamp(color,0,1);
	color = mix(vec3(dot(color,vec3(0.299,0.587,0.114))), color,2);//saturation

	color = clamp(color,0,1);
	color = 1.1*(color-0.5) + 0.5 ; //contrast

	return color;
}

void main()
{
	vec3 m_color = vec3(1);// need to change it as it will be the terrain color tint

	vec2 texture_size = textureSize(u_HeightMap,0);	//get texture dimension
	vec3 Normal = TBN(fs_data.TexCoord , vec2(1/texture_size.x)) * texture(u_Normal , fs_data.TexCoord * u_Tiling).rgb;
	//vec3 Normal = CalculateNormal(fs_data.TexCoord , vec2(1/texture_size.x));
	Normal = normalize(Normal);
	//vec3 Normal = CalculateNormal(fs_data.TexCoord , vec2(1/2048.0));
	float Height = texture(u_HeightMap,fs_data.TexCoord).r * HEIGHT_SCALE;
	float y = (Height)/HEIGHT_SCALE;

	alpha = texture(u_Roughness , fs_data.TexCoord * u_Tiling).r ;
	vec3 DirectionalLight_Direction = normalize(DirectionalLight_Direction );//for directional light as it has no concept of position
	NdotL = max(dot(Normal,DirectionalLight_Direction),0.0001);
	vec3 EyeDirection = normalize(u_CameraPos - grass_data.Pos.xyz);

	vec4 vert_pos = view * grass_data.Pos; //get depth value(z value) in the camera-view space
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

	VertexPosition_LightSpace = MatrixShadow[level] * grass_data.Pos;

	//shadows
	float shadow = CalculateShadow(level);


	//diffuse_environment reflections
	vec3 Light_dir_i = reflect(-EyeDirection,Normal);
	//vec3 diffuse_environmentCol = texture(diffuse_env,Light_dir_i).xyz * (1.0 - alpha) ;

	vdoth = max(dot( EyeDirection, normalize( EyeDirection + DirectionalLight_Direction)) ,0.0);//for directional light
	ks = Fresnel(vdoth);
	kd = vec3(1.0) - ks;
	kd *= (1.0 - Metallic);

	vec3 IBL_diffuse =  texture(diffuse_env,Normal).rgb * kd; //sampling the irradiance map
	vec3 BRDFintegration =  ks*alpha + max(dot(Normal,DirectionalLight_Direction),0.001) ;// we preapare the multiplication factor by the roughness and the NdotL value
	vec3 IBL_specular = textureLod(specular_env,Light_dir_i , MAX_MIP_LEVEL * alpha).rgb * BRDFintegration ; //sample the the environment map at varying mip level
	
	vec4 coordinate = u_ProjectionView * grass_data.Pos;
	coordinate.xyz /= coordinate.w;
	coordinate.xyz = coordinate.xyz*0.5 + 0.5;
	//ambiance
		vec3 ambiant = (IBL_diffuse + IBL_specular)* texture(u_Albedo, fs_data.TexCoord * u_Tiling).xyz * m_color.xyz;// *  texture(SSAO,coordinate.xy).r;

	PBR_Color += ( (kd * texture(u_Albedo, fs_data.TexCoord * u_Tiling).xyz * m_color.xyz  / PI) + SpecularBRDF(DirectionalLight_Direction , EyeDirection , Normal) ) * (shadow * SunLight_Color * SunLight_Intensity) * max(dot(Normal,DirectionalLight_Direction), 0.0) ; //for directional light (no attenuation)

	
	//if(y<WaterLevel)
	//{
	//	Fcolor = vec4(0.6,1,0,1);
	//}
	//else if(y>WaterLevel && y<HillLevel)
	//{
	//	float value = (y-WaterLevel)/(HillLevel-WaterLevel);
	//	Fcolor = vec4(mix(green,mud,value).rgb,1.0);
	//}
	//else if(y>HillLevel && y<MountainLevel)
	//{
	//	float value = (y-HillLevel)/(MountainLevel - HillLevel);
	//	Fcolor = vec4(mix(mud,grey,value).rgb,1.0);
	//}
	//else if(y>MountainLevel)
	//{
	//	Fcolor = grey;
	//}
	
	PBR_Color += ambiant;
	
	PBR_Color = ColorCorrection(PBR_Color);

	color = vec4(PBR_Color,1.0);
	//color = vec4(Normal,1.0);
}