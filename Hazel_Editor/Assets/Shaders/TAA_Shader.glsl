//#shader vertex
#version 410 core

layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 tcord;

out vec2 tex_coord;
void main()
{
	gl_Position = pos;
	tex_coord = tcord.xy;
}

//#shader fragment
#version 410 core

layout (location = 0) out vec4 color;

in vec2 tex_coord;

uniform sampler2D History_Buffer;
uniform sampler2D Current_Buffer;
uniform sampler2D gVelocity;
uniform sampler2D Depth_Buffer;

vec2 ClosestFragment() //returns the coordinate having lowest depth value in a 3x3 neighbour pixels
{
	float depth = texture(Depth_Buffer, tex_coord).x;
	vec2 pixelDist = 1.0 / vec2(textureSize(Depth_Buffer,0));
	float depths[8];
	vec2 closestCoord[8];

	depths[0] = texture(Depth_Buffer, tex_coord + vec2(-pixelDist.x,0.0)).x;
	depths[1] = texture(Depth_Buffer, tex_coord + vec2(pixelDist.x,0.0)).x;
	depths[2] = texture(Depth_Buffer, tex_coord + vec2(0.0,pixelDist.y)).x;
	depths[3] = texture(Depth_Buffer, tex_coord + vec2(0.0,-pixelDist.y)).x;
	depths[4] = texture(Depth_Buffer, tex_coord + pixelDist).x;
	depths[5] = texture(Depth_Buffer, tex_coord - pixelDist).x;
	depths[6] = texture(Depth_Buffer, tex_coord + vec2(pixelDist.x,-pixelDist.y)).x;
	depths[7] = texture(Depth_Buffer, tex_coord + vec2(-pixelDist.x,pixelDist.y)).x;

	closestCoord[0] = tex_coord + vec2(-pixelDist.x,0.0);
	closestCoord[1] = tex_coord + vec2(pixelDist.x,0.0);
	closestCoord[2] = tex_coord + vec2(0.0,pixelDist.y);
	closestCoord[3] = tex_coord + vec2(0.0,-pixelDist.y);
	closestCoord[4] = tex_coord + pixelDist;
	closestCoord[5] = tex_coord - pixelDist;
	closestCoord[6] = tex_coord + vec2(pixelDist.x,-pixelDist.y);
	closestCoord[7] = tex_coord + vec2(-pixelDist.x,pixelDist.y);

	float minDepth = depth; vec2 closestFragCoord;
	for(int i=0;i<8;i++)
	{
		if(depths[i]<minDepth)
		{
			minDepth = depths[i];
			closestFragCoord = closestCoord[i];
		}
	}
	return closestFragCoord;
}

void main()
{
	vec3 current_image = texture(Current_Buffer,tex_coord).rgb;
	vec2 pixelDist = 1.0 / vec2(textureSize(Current_Buffer,0));
	vec3 leftColor = texture(Current_Buffer,tex_coord + vec2(-pixelDist.x,0.0)).rgb;
	vec3 rightColor = texture(Current_Buffer,tex_coord + vec2(pixelDist.x,0.0)).rgb;
	vec3 topColor = texture(Current_Buffer,tex_coord + vec2(0.0,pixelDist.y)).rgb;
	vec3 bottomColor = texture(Current_Buffer,tex_coord + vec2(0.0,-pixelDist.y)).rgb;
	vec3 min_color = min(current_image, min(leftColor, min(rightColor, min(topColor, bottomColor))));
	vec3 max_color = max(current_image, max(leftColor, max(rightColor, max(topColor, bottomColor))));
	
	vec2 closestFragCoord = ClosestFragment();
	vec2 prevPixel = tex_coord - texture(gVelocity,closestFragCoord).xy; //sample velocity from closest depth fragment
	float mixfactor = 0.9;
	vec3 old_image = clamp(texture(History_Buffer,prevPixel).rgb,min_color,max_color); //box-clamp to reduce ghosting
	color = vec4(mix(current_image,old_image,mixfactor),1.0);
}