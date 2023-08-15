#shader vertex
#version 410 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 cord;
layout (location = 2) in vec3 normal;

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
const float MIN_TESS_LEVEL = 2;
const float MAX_CAM_DIST = 50;
const float MIN_CAM_DIST = 50;

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
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	tcs_data[gl_InvocationID].TexCoord_TCS = TexCoord[gl_InvocationID];

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

}



#shader tessellation evaluation
#version 410 core
layout (quads,fractional_even_spacing ,ccw) in;

in TCS_Data
{
	vec2 TexCoord_TCS;
} tcs_data[];

uniform sampler2D u_HeightMap;
uniform mat4 u_ProjectionView;
uniform mat4 u_Model;
uniform float HEIGHT_SCALE;

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
	vec4 oldPos = u_ProjectionView * u_Model * Interpolate(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position, gl_in[3].gl_Position);
	vec2 texCoord = Interpolate(tcs_data[0].TexCoord_TCS, tcs_data[1].TexCoord_TCS, tcs_data[2].TexCoord_TCS, tcs_data[3].TexCoord_TCS);

	vec4 newPos = u_ProjectionView * u_Model * vec4(0,texture(u_HeightMap,texCoord).r*HEIGHT_SCALE,0,1); //as proj_view and model matrix is same for all vertex
	gl_Position = oldPos + newPos;
}



#shader geometry
#version 410 core
layout (triangles) in;
layout (line_strip,max_vertices = 3) out;

void main()
{
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();

	gl_Position = gl_in[1].gl_Position;
	EmitVertex();

	gl_Position = gl_in[2].gl_Position;
	EmitVertex();
}



#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

void main()
{
	color = vec4(1.0,1.0,1.0,1.0);
}