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
uniform mat4 u_oldProjectionView;
uniform mat4 u_Model;
uniform mat4 u_View;
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
	vec4 m_curPos;
	vec4 m_oldPos;
	vec3 Normal;
}grass_data;


void main()
{
	vec4 VertexPos0 = u_Model * gl_in[0].gl_Position; // in world space
	vec4 VertexPos1 = u_Model * gl_in[1].gl_Position; // in world space
	vec4 VertexPos2 = u_Model * gl_in[2].gl_Position; // in world space

	vec4 clip_space;
	clip_space = u_ProjectionView * VertexPos0;
	gl_Position = clip_space;
	fs_data.TexCoord = gs_data[0].TexCoord;
	grass_data.m_curPos = clip_space;
	grass_data.m_oldPos = u_oldProjectionView * VertexPos0; //oldPos to create the velocity buffer
	grass_data.Pos = u_View * VertexPos0; //view space
	EmitVertex();

	clip_space = u_ProjectionView * VertexPos1;
	gl_Position = clip_space;
	fs_data.TexCoord = gs_data[1].TexCoord;
	grass_data.m_curPos = clip_space;
	grass_data.m_oldPos = u_oldProjectionView * VertexPos1; //oldPos to create the velocity buffer
	grass_data.Pos = u_View * VertexPos1; //view space
	EmitVertex();

	clip_space = u_ProjectionView * VertexPos2;
	gl_Position = clip_space;
	fs_data.TexCoord = gs_data[2].TexCoord;
	grass_data.m_curPos = clip_space;
	grass_data.m_oldPos = u_oldProjectionView * VertexPos2; //oldPos to create the velocity buffer
	grass_data.Pos = u_View * VertexPos2; //view space
	EmitVertex();	
}



#shader fragment
#version 410 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gColor;
layout (location = 3) out vec4 gRoughnessMetallic;
layout (location = 4) out vec4 gVelocity;

in FS_Data
{
	vec2 TexCoord;
}fs_data;

in Grass
{
	vec4 Pos;
	vec4 m_curPos;
	vec4 m_oldPos;
	vec3 Normal;	
}grass_data;

uniform float WaterLevel;
uniform float HillLevel;
uniform float MountainLevel;
uniform float u_Intensity;
uniform float HEIGHT_SCALE = 1000;
uniform float u_Tiling;

uniform sampler2D u_HeightMap;
uniform sampler2D u_Albedo;
uniform sampler2D u_Roughness;
uniform sampler2D u_Normal;
uniform mat4 u_View;

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
	vec2 texture_size = textureSize(u_HeightMap,0);	//get texture dimension
	vec3 Normal = mat3(u_View) * TBN(fs_data.TexCoord , vec2(1/texture_size.x)) * texture(u_Normal , fs_data.TexCoord * u_Tiling).rgb;
	
	Normal = normalize(Normal);
	
	gPosition = vec4(grass_data.Pos.xyz , 1.0);
	gNormal = vec4(Normal.xyz,1.0);
	gColor = vec4(GammaCorrection(texture(u_Albedo,fs_data.TexCoord * u_Tiling).xyz) , 1.0);
	gRoughnessMetallic = vec4(texture(u_Roughness,fs_data.TexCoord * u_Tiling).x,0,0 , 1.0);
	gVelocity = vec4(CalculateVelocity(grass_data.m_curPos,grass_data.m_oldPos),0,1.0);
}