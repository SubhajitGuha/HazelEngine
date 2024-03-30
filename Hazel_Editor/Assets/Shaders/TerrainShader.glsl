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
const float MAX_TESS_LEVEL = 32;
const float MIN_TESS_LEVEL = 4;
const float MAX_CAM_DIST = 1000;
const float MIN_CAM_DIST = 0;

in vec2 TexCoord[];
out TCS_Data
{
	vec2 TexCoord_TCS;
} tcs_data[];

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
		
		float dist01 = clamp( (abs(p1.z/p1.w) - MIN_CAM_DIST) / (MAX_CAM_DIST-MIN_CAM_DIST), 0.0, 1.0);
		float dist02 = clamp( (abs(p2.z/p2.w) - MIN_CAM_DIST) / (MAX_CAM_DIST-MIN_CAM_DIST), 0.0, 1.0);
		float dist03 = clamp( (abs(p3.z/p3.w) - MIN_CAM_DIST) / (MAX_CAM_DIST-MIN_CAM_DIST), 0.0, 1.0);
		float dist04 = clamp( (abs(p4.z/p4.w) - MIN_CAM_DIST) / (MAX_CAM_DIST-MIN_CAM_DIST), 0.0, 1.0);

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
uniform vec3 camPos; //in ws

in GS_Data
{
	vec2 TexCoord;
}gs_data[];

out FS_Data
{
	vec2 TexCoord;
}fs_data;

out vertexAttrib
{
	vec4 m_curPos;
	vec4 m_oldPos;
}vertex_data;


void main()
{
	for(int i=0;i<3;i++)
	{
		vec4 VertexPos = u_Model * gl_in[i].gl_Position; // in world space		
		vec4 clip_space = u_ProjectionView * VertexPos;
		gl_Position = clip_space;
		fs_data.TexCoord = gs_data[i].TexCoord;
		vertex_data.m_curPos = clip_space;
		vertex_data.m_oldPos = u_oldProjectionView * VertexPos; //oldPos to create the velocity buffer
		EmitVertex();
	}
}



#shader fragment
#version 410 core
layout (location = 0) out vec4 gNormal;
layout (location = 1) out vec4 gVelocity;
layout (location = 2) out vec4 gColor;
layout (location = 3) out vec4 gRoughnessMetallic;

in FS_Data
{
	vec2 TexCoord;
}fs_data;

in vertexAttrib
{
	vec4 m_curPos;
	vec4 m_oldPos;	
}vertex_data;

uniform float WaterLevel;
uniform float HillLevel;
uniform float MountainLevel;
uniform float u_Intensity;
uniform float HEIGHT_SCALE = 1000;
uniform float u_Tiling;

uniform sampler2D u_HeightMap;
uniform sampler2DArray u_Albedo;
uniform sampler2DArray u_Roughness;
uniform sampler2DArray u_Normal;
uniform sampler2DArray u_Masks;

uniform mat4 u_View;

const int MaxNumTextures = 5; //defines the total number of textures that I can pass and store

//https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/chapter5-andersson-terrain-rendering-in-frostbite.pdf
vec3 CalculateNormal(vec2 texCoord , vec2 texelSize)
{
	float left = texture(u_HeightMap,texCoord + vec2(-texelSize.x,0.0)).r * HEIGHT_SCALE  *2.0 - HEIGHT_SCALE; //-height to +height
	float right = texture(u_HeightMap,texCoord + vec2(texelSize.x,0.0)).r * HEIGHT_SCALE *2.0 - HEIGHT_SCALE;
	float up = texture(u_HeightMap,texCoord + vec2(0.0 , texelSize.y)).r * HEIGHT_SCALE *2.0 - HEIGHT_SCALE;
	float down = texture(u_HeightMap,texCoord + vec2(0.0 , -texelSize.y)).r * HEIGHT_SCALE *2.0 - HEIGHT_SCALE;

	return normalize(vec3(left-right,2.0,down-up));
}

mat3 TBN(vec3 N)
{	
	vec3 T = cross(vec3(1.0,0.0,0.0),N); //tangent of terrain wrt height map aligned with terrain
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

vec3 albedo_maps[MaxNumTextures];
vec3 normal_maps[MaxNumTextures];
vec3 roughness_maps[MaxNumTextures];
void main()
{
	/*
		Terrain Layer Format:
		Layer0: Grass Texture; //base texture
		Layer1: Another Grass1; //masked texture
		Layer2: Another Grass2; //masked texture
		Layer3: Another Grass3; //masked texture
		......
		Layer(K): Cliff/Rock Texture //procedural masked textures(created/masked based on terrain data like slope);
		Layer(K+1): procedural masked Texture2 //procedural masked textures(created/masked based on terrain data like slope);
		.......
	*/
	vec3 total_albedo;
	vec3 total_normal;
	vec3 total_roughness;

	int numMaskMaps = textureSize(u_Masks,0).z;
	int numTextureMaps = textureSize(u_Albedo,0).z;

	//initilize the albedo, normal, roughness with the first texture in the array
	total_albedo  = texture(u_Albedo, vec3(fs_data.TexCoord * u_Tiling,0)).rgb;
	total_normal  = texture(u_Normal, vec3(fs_data.TexCoord * u_Tiling,0)).rgb;
	total_normal = total_normal*2.0 - 1.0;
	total_roughness  = texture(u_Roughness, vec3(fs_data.TexCoord * u_Tiling,0)).rgb;

	//collect the other textures and save them
	for(int i=0;i<numTextureMaps-1;i++)
	{
		albedo_maps[i]  = texture(u_Albedo, vec3(fs_data.TexCoord * u_Tiling,i+1)).rgb;
		normal_maps[i]  = texture(u_Normal, vec3(fs_data.TexCoord * u_Tiling,i+1)).rgb;
		normal_maps[i] = normal_maps[i]*2.0 - 1.0; //convert to -1.0 - 1.0
		roughness_maps[i]  = texture(u_Roughness, vec3(fs_data.TexCoord * u_Tiling,i+1)).rgb;
	}

	//blend the textures based on masks using linear-Interpolate
	for(int i=0;i<numMaskMaps;i++)
	{
		float weight = texture(u_Masks, vec3(fs_data.TexCoord ,i)).r;
		total_albedo = mix(total_albedo, albedo_maps[i], weight);
		total_normal = mix(total_normal,normal_maps[i], weight);
		total_roughness = mix(total_roughness, roughness_maps[i], weight);
	}

	vec2 texture_size = textureSize(u_HeightMap,0);	//get texture dimension

	vec3 Normal = CalculateNormal(fs_data.TexCoord , vec2(1.0/texture_size.x));
	float slope = 1.0-Normal.y; //slope is 1.0-normal.y as suggested by the dice-terrainRendering paper page 43
	mat3 tbn = TBN(Normal);
	slope = clamp((slope-0.5)*10.0+0.5,0.0,1.0);

	//for the procedural masked textures (for now there is only slope)
	for(int i=numMaskMaps;i<numTextureMaps-1.0;i++)
	{
		total_normal = mix(total_normal,normal_maps[i],slope);
		total_albedo = mix(total_albedo,albedo_maps[i],slope);
		total_roughness = mix(total_roughness,roughness_maps[i],slope);
	}
	Normal = mat3(u_View) * tbn * total_normal;
	

	Normal = normalize(Normal);
	
	gNormal = vec4(Normal.xyz,1.0);
	gVelocity = vec4(CalculateVelocity(vertex_data.m_curPos,vertex_data.m_oldPos),0,1.0);
	gColor = vec4(GammaCorrection(total_albedo) , 1.0);
	gRoughnessMetallic = vec4(total_roughness.r,0,0 , 1.0);
}