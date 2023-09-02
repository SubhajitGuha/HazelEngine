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

uniform float HEIGHT_SCALE;
uniform vec3 camPos; //in ws
uniform mat4 u_Model;
uniform mat4 u_View;
uniform sampler2D u_HeightMap;

void main()
{
	if(gl_InvocationID == 0)
	{
		//float Height1 = texture(u_HeightMap,tcs_data[0].TexCoord_TCS).r * HEIGHT_SCALE;
		//vec4 newPos1 = vec4(0,Height1,0,0);
		//float Height2 = texture(u_HeightMap,tcs_data[1].TexCoord_TCS).r * HEIGHT_SCALE;
		//vec4 newPos2 = vec4(0,Height2,0,0);
		//float Height3 = texture(u_HeightMap,tcs_data[2].TexCoord_TCS).r * HEIGHT_SCALE;
		//vec4 newPos3 = vec4(0,Height3,0,0);
		//float Height4 = texture(u_HeightMap,tcs_data[3].TexCoord_TCS).r * HEIGHT_SCALE;
		//vec4 newPos4 = vec4(0,Height4,0,0);


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
layout (line_strip,max_vertices = 6) out;

uniform mat4 u_ProjectionView;
uniform mat4 u_Model;
uniform float HEIGHT_SCALE;
uniform float HillLevel;
uniform sampler2D u_perlinNoise;

in GS_Data
{
	vec2 TexCoord;
}gs_data[];

void main()
{
	vec4 VertexPos0_ws = u_Model * gl_in[0].gl_Position;
	vec4 VertexPos1_ws = u_Model * gl_in[1].gl_Position;
	vec4 VertexPos2_ws = u_Model * gl_in[2].gl_Position;

	gl_Position = u_ProjectionView * VertexPos0_ws;
	EmitVertex();

	gl_Position = u_ProjectionView * VertexPos1_ws;
	EmitVertex();

	gl_Position = u_ProjectionView * VertexPos2_ws;
	EmitVertex();
}



#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

void main()
{
	color = vec4(1.0);
}