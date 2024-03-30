#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 cord;

out vec2 tcord;
out vec3 m_pos;

void main()
{
	gl_Position = pos;
	m_pos = pos.xyz;
    tcord = cord.xy;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

uniform sampler2D u_sceneDepth;
uniform sampler2D u_sceneColor;
uniform mat4 u_Projection;
uniform mat4 u_View;
uniform vec3 u_CamDir;
uniform float u_density;
uniform float u_gradient;
uniform vec3 u_fogColor;
uniform float u_fogTop;
uniform float u_fogEnd;
uniform vec3 u_CamPos;
uniform vec3 u_sunDir;
uniform vec3 u_ViewDir;

in vec2 tcord;
in vec3 m_pos;


vec4 GetWorldPosition(vec2 texCoord)
{
	float z = texture2D(u_sceneDepth, tcord).x;
	vec4 clip_space = vec4(texCoord*2.0-1.0, z*2.0-1.0, 1.0);
	vec4 viewSpace = inverse(u_Projection) * clip_space;
	viewSpace/=viewSpace.w;
	vec4 worldSpace = inverse(u_View)*viewSpace;
	return worldSpace;
}
float GetFogDensity()
{
	vec3 proj_CameraPos = u_CamPos;
	proj_CameraPos.y=0.0;

	vec3 WorldPos = GetWorldPosition(tcord).xyz;

	vec3 ws_PixelPos = WorldPos;
	ws_PixelPos.y = 0;

	float DeltaD = length(proj_CameraPos-ws_PixelPos) / u_fogEnd;

	float DeltaY = 0.0;
	float DensityIntegral = 0.0;

	if(u_CamPos.y > u_fogTop) //camera above the fog
	{
		if(WorldPos.y < u_fogTop) //pixel is inside the fog
		{
			DeltaY = (u_fogTop - WorldPos.y) / u_fogTop;
			DensityIntegral = DeltaY * DeltaY*0.5;
		}
		//else if pixel is above the fog do nothing
	}
	else if(WorldPos.y < u_fogTop) //pixel is inside fog
	{
		DeltaY = abs(u_CamPos.y - WorldPos.y)/ u_fogTop;
		float deltaCamera = (u_fogTop - u_CamPos.y)/u_fogTop;
		float densityIntegralCamera = deltaCamera * deltaCamera * 0.5;
		float deltaPixel = (u_fogTop - WorldPos.y)/u_fogTop;
		float densityIntegralPixel = deltaPixel * deltaPixel * 0.5;
		DensityIntegral = abs(densityIntegralCamera-densityIntegralPixel);
	}
	else //pixel is above the fog
	{
		DeltaY = (u_fogTop-u_CamPos.y)/u_fogTop;
		DensityIntegral = DeltaY * DeltaY * 0.5;
	}

	float fogDensity = 0.0;
	if(DeltaY !=0.0)
		fogDensity = (sqrt(1.0 + ((DeltaD/DeltaY)*(DeltaD/DeltaY)))) * DensityIntegral;
	return fogDensity;
}

void main()
{
	float fogDensity = pow(GetFogDensity(),u_density);
	vec3 skyColor = u_fogColor;

	vec3 pixelWorldPos = GetWorldPosition(tcord).xyz;
	float sunAmount = max( dot(normalize(pixelWorldPos-u_CamPos), normalize(-u_sunDir)), 0.0 );
	float scattering_gradient = exp(pow(sunAmount,64)*0.15); //controlls the amount of scattering in the sun direction
    vec3  fogColor  = mix( vec3(0.5,0.6,0.7), // blue
                           vec3(1.0,0.8,0.5), // yellow
                           pow(sunAmount,4.0) );
	vec3 sceneColor = fogColor * (1.0 - exp(-fogDensity)) + texture(u_sceneColor,tcord).rgb * exp(-fogDensity);
	color = vec4(sceneColor * scattering_gradient, 1.0);
}