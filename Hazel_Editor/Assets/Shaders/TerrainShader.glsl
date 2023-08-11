#shader vertex
#version 410 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 cord;
layout (location = 2) in vec3 normal;

uniform mat4 u_ProjectionView;
uniform mat4 u_Model;
uniform float u_maxTerrainHeight;
out vec3 vertexNormal;

void main()
{
	gl_Position = u_ProjectionView * u_Model * vec4(pos,1);
	vertexNormal = normalize(normal);
}


#shader fragment
#version 410 core
layout (location = 0) out vec4 color;

in vec3 vertexNormal;
uniform vec3 LightDir;
uniform vec3 CameraDir;
void main()
{
	color = vec4(vertexNormal,1);
}