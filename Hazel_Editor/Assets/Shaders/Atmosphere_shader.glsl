#shader vertex
#version 410 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 cord;

out vec2 tcord;
out vec4 m_pos;

uniform mat4 u_Model;

void main()
{
	gl_Position = pos;
	tcord = cord.xy;
	//m_pos = u_Model * pos;
	m_pos = pos;
}

#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

in vec2 tcord;
in vec4 m_pos;

uniform mat4 u_ProjectionView;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform vec3 camera_pos;
uniform vec3 view_dir;
uniform float atmosphere_radius;
uniform sampler2D depthTex;
uniform sampler2D sceneTex;

uniform vec3 sun_direction;
uniform float densityFalloff;

#define FLT_MAX 3.402823466e+38

float numScatterPoints = 20;
float planetRadius = 10;
vec3 SphereOrigin = vec3(0,0,0);
float bias = 0.1;

vec2 RaySphere(vec3 sphereCentre, float sphereRadius, vec3 rayOrigin, vec3 rayDir)
{
	vec3 offset = rayOrigin - sphereCentre;
	float a= 1;
	float b = 2 * dot(offset,rayDir);
	float c = dot(offset ,offset) - sphereRadius * sphereRadius;
	float d = b * b - 4 * a * c;

	if(d > 0)
	{
		float s = sqrt(d);
		float distToSphereNear = max(0, (-b-s)/(2*a));
		float distToSphereFar = (-b+s)/(2*a);

		if(distToSphereFar >= 0)
			return vec2(distToSphereNear, distToSphereFar - distToSphereNear);
	}
	return vec2(FLT_MAX,0);
}

float densityAtPoint(vec3 densitySamplePoint)
{
	float heightAboveSurface = length(densitySamplePoint - SphereOrigin.xyz)- planetRadius;
	float height01 = heightAboveSurface/(atmosphere_radius - planetRadius);
	float localDensity = exp(-height01 * densityFalloff) * (1- height01);
	return localDensity;
}

float opticalDepth(vec3 rayOrigin, vec3 rayDir, float rayLen)
{
	vec3 densitySamplePoint = rayOrigin;
	float stepSize = rayLen/ (numScatterPoints - 1);
	float opticalDepth = 0;

	for(int i=0; i<numScatterPoints; i++)
	{
		float localDensity = densityAtPoint(densitySamplePoint);
		opticalDepth += localDensity * stepSize;
		densitySamplePoint += rayDir * stepSize;
	}
	return opticalDepth;
}

float calcLight(vec3 rayOrigin, vec3 rayDir, float rayLen)
{
	vec3 inScatterPoint = rayOrigin;
	float stepSize = rayLen/(numScatterPoints-1);
	float inScatteredLight = 0;

	for(int i=0; i<numScatterPoints; i++)
	{
		float sunRayLen = RaySphere(SphereOrigin.xyz, atmosphere_radius, inScatterPoint, -sun_direction).y;
		float sunRayOpticalDepth = opticalDepth(inScatterPoint , -sun_direction, sunRayLen);
		float viewRayOpticalDepth = opticalDepth(inScatterPoint, -view_dir, stepSize *i);

		float transmittance = exp(-(sunRayOpticalDepth + viewRayOpticalDepth));
		float localDensity = densityAtPoint(inScatterPoint);

		inScatteredLight += localDensity * transmittance * stepSize;
		inScatterPoint += rayDir * stepSize;
	}

	return inScatteredLight;
}

void main()
{
	vec4 coordinate = m_pos;//u_ProjectionView * m_pos;
	coordinate.xyz /= coordinate.w;
	coordinate.xyz = coordinate.xyz*0.5 + 0.5;

	vec4 originalImage = texture(sceneTex, tcord);
	vec4 worldPos = inverse(u_View) * texture(depthTex,coordinate.xy).xyzw;
	worldPos.xyz/= worldPos.w;

	vec4 wp = inverse(u_Projection * mat4(mat3(u_View))) * m_pos;//to worldspace

	vec3 rayOrigin = camera_pos;
	vec3 rayDir = wp.xyz;

	vec2 hitInfo = RaySphere(SphereOrigin.xyz , atmosphere_radius, rayOrigin, normalize(rayDir));
	float distToAtms = hitInfo.x;
	float distThAtms = min(hitInfo.y , abs(worldPos.z)*length(rayDir) - distToAtms);

	
	if(distThAtms > 0)
	{
		vec3 pointInAtmosphere = rayOrigin + rayDir * distToAtms;
		float light = calcLight(pointInAtmosphere, rayDir, distThAtms);
		color = originalImage * (1-light) + light;
		return;
	}

	color = originalImage ;
	//color = vec4(distThAtms/(atmosphere_radius*2));
	

}