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
const float MAX_TESS_LEVEL = 64;
const float MIN_TESS_LEVEL = 2;
const float MAX_CAM_DIST = 200;
const float MIN_CAM_DIST = 10;

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
	vec4 newPos = vec4(0,Height,0,1); //as proj_view and model matrix is same for all vertex
	
	gl_Position = oldPos + newPos;
}


#shader geometry
#version 410 core
layout (triangles) in;
layout (triangle_strip,max_vertices = 6) out;

uniform mat4 u_ProjectionView;
uniform mat4 u_Model;
uniform float HEIGHT_SCALE;
uniform float HillLevel;
uniform sampler2D randFloat;
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
	vec3 Pos;
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
	grass_data.Pos = vec3(0.0);
	EmitVertex();

	gl_Position = u_ProjectionView * VertexPos1_ws;
	fs_data.TexCoord = gs_data[1].TexCoord;
	grass_data.Pos = vec3(0.0);
	EmitVertex();

	gl_Position = u_ProjectionView * VertexPos2_ws;
	fs_data.TexCoord = gs_data[2].TexCoord;
	grass_data.Pos = vec3(0.0);
	EmitVertex();

	EndPrimitive();
	
	float y = gl_in[1].gl_Position.y/HEIGHT_SCALE;
	if(y>HillLevel)
		return;

	//Render Grass	
	vec3 n1 = cross((gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz), (gl_in[2].gl_Position.xyz - gl_in[1].gl_Position.xyz));
	//vec3 n2 = cross((gl_in[0].gl_Position.xyz - gl_in[2].gl_Position.xyz), (gl_in[1].gl_Position.xyz - gl_in[2].gl_Position.xyz));
	
	float seed = gs_data[1].TexCoord.x + gs_data[1].TexCoord.y;
	float angleX = gs_data[1].TexCoord.x * 2048.0;
	float angleY = gs_data[1].TexCoord.y * 2048.0;
	vec4 position = VertexPos1_ws + vec4(normalize(mat3(u_Model) * n1)*5.0 * random(seed),0.0) ;//+ vec4(sin(Time*angleX),0,sin(Time*angleY),0.0) * 10.0;
	gl_Position = u_ProjectionView * position;
	grass_data.Pos = gl_Position.xyz;
	fs_data.TexCoord = gs_data[1].TexCoord;
	EmitVertex();

	gl_Position = u_ProjectionView * VertexPos1_ws;
	fs_data.TexCoord = gs_data[1].TexCoord;
	grass_data.Pos = gl_Position.xyz;
	EmitVertex();

	gl_Position = u_ProjectionView * VertexPos2_ws;
	fs_data.TexCoord = gs_data[2].TexCoord;
	grass_data.Pos = gl_Position.xyz;
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
	vec3 Pos;
	vec3 Normal;
}grass_data;

uniform float WaterLevel;
uniform float HillLevel;
uniform float MountainLevel;
uniform vec3 u_CameraPos;
uniform vec3 u_LightDir;
uniform float u_Intensity;
uniform sampler2D u_HeightMap;
uniform float HEIGHT_SCALE = 1000;

vec3 CalculateNormal(vec2 texCoord , vec2 texelSize)
{
	float left = texture(u_HeightMap,texCoord + vec2(-texelSize.x,0.0)).r * HEIGHT_SCALE  *2.0 - HEIGHT_SCALE;
	float right = texture(u_HeightMap,texCoord + vec2(texelSize.x,0.0)).r * HEIGHT_SCALE *2.0 - HEIGHT_SCALE;
	float up = texture(u_HeightMap,texCoord + vec2(0.0 , texelSize.y)).r * HEIGHT_SCALE *2.0 - HEIGHT_SCALE;
	float down = texture(u_HeightMap,texCoord + vec2(0.0 , -texelSize.y)).r * HEIGHT_SCALE *2.0 - HEIGHT_SCALE;

	return normalize(vec3(down-up,2.0,left-right));
}

void main()
{
	vec2 texture_size = textureSize(u_HeightMap,0);	//get texture dimension
	vec3 Normal = CalculateNormal(fs_data.TexCoord , vec2(1/2048.0));
	float Height = texture(u_HeightMap,fs_data.TexCoord).r * HEIGHT_SCALE;
	float y = (Height)/HEIGHT_SCALE;
	vec4 grey = vec4(1,1,1,1);
	vec4 green = vec4(0.2,1,0,1);
	vec4 mud = vec4(0.8,0.6,0.02,1);
	vec4 Fcolor = vec4(0);

	if(y<WaterLevel)
	{
		Fcolor = vec4(0.6,1,0,1);
	}
	else if(y>WaterLevel && y<HillLevel)
	{
		float value = (y-WaterLevel)/(HillLevel-WaterLevel);
		Fcolor = vec4(mix(green,mud,value).rgb,1.0);
	}
	else if(y>HillLevel && y<MountainLevel)
	{
		float value = (y-HillLevel)/(MountainLevel - HillLevel);
		Fcolor = vec4(mix(mud,grey,value).rgb,1.0);
	}
	else if(y>MountainLevel)
	{
		Fcolor = grey;
	}
	
	Fcolor = vec4(Fcolor.rgb * dot(Normal,normalize(u_LightDir)) * u_Intensity ,1.0);
	Fcolor += vec4(0.3);
	Fcolor = vec4(1.0) - exp(-Fcolor * 2);//exposure
	Fcolor = pow(Fcolor, vec4(1.0/2.2)); //Gamma correction
	color = vec4(Fcolor.rgb,1.0);
	//color = vec4(Normal,1.0);
	//color = vec4(vec3(nextFloat(int(Height) + nextInt(int(Height)) + int(fs_data.TexCoord.x*2048.0) +int(fs_data.TexCoord.y*2048.0))),1.0);
}