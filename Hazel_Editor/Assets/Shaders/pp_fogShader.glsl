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
uniform float u_nearPlane;
uniform float u_farPlane;
uniform float u_density;
uniform float u_gradient;
uniform vec3 u_fogColor;

in vec2 tcord;
in vec3 m_pos;

void main()
{
	float z_b = texture2D(u_sceneDepth, tcord).x;

    float z_n = 2.0 * z_b - 1.0;
    float z_e = 2.0 * u_nearPlane * u_farPlane / (u_farPlane + u_nearPlane - z_n * (u_farPlane - u_nearPlane));
	float visibility = exp(-pow(z_e*u_density , u_gradient));
	visibility = clamp(visibility, 0, 1);
	vec3 skyColor = u_fogColor;

	color = vec4(mix(skyColor, texture(u_sceneColor,tcord).rgb ,visibility), 1.0);
}