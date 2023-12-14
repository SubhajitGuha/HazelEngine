//#shader compute
#version 460 core
#define MIN -9999999
#define NUM_SPHERES 2

layout (local_size_x = 8, local_size_y = 8) in;

uniform layout(binding = 1, rgba8) writeonly image2D FinalImage;
uniform float viewport_w;
uniform float viewport_h;
uniform float focal_length;
uniform vec3 camera_pos;
uniform vec3 camera_viewdir;
uniform vec3 light_dir;
uniform mat4 mat_view;
uniform mat4 mat_proj;

struct Sphere
{
	vec3 centre;
	float radius;
};

struct Ray
{
	vec3 origin;
	vec3 dir;
};

//check whether the ray hits the sphere or not
float rayHit(Sphere sh,Ray ray)
{
	vec3 ac = ray.origin - sh.centre;
	float a = dot(ray.dir,ray.dir);
	float b = 2*dot(ray.dir,ac);
	float c = dot(ac,ac)-pow(sh.radius,2);
	float discriminant = b*b - 4*a*c;

	if(discriminant<0)
		return MIN;
	else
		return (-b-sqrt(discriminant))/2*a;
}


void main()
{
	ivec2 uv = ivec2(gl_GlobalInvocationID.xy);

	vec2 image_res = gl_NumWorkGroups.xy*gl_WorkGroupSize.xy;
	vec2 coord = gl_GlobalInvocationID.xy/image_res;
	coord = coord*2.0-1.0;
	vec4 target = inverse(mat_proj) * vec4(coord,1,1); //to view space from clip-space
	vec4 color = vec4(coord,0,1.0);
	
	Ray ray;
	ray.origin = camera_pos;//in world space
	ray.dir = normalize(vec3(inverse(mat_view)*vec4(target.xyz/target.w,0))); //ray dir in world space

	Sphere sphere[2];
	sphere[0].centre = vec3(0,1,0);
	sphere[0].radius = 10.0;
	
	sphere[1].centre = vec3(0,-110,0);
	sphere[1].radius = 100.0;

	for(int i=0;i<NUM_SPHERES;i++)
	{
		float t = rayHit(sphere[i],ray);
		if(t > MIN)
		{
			vec3 N = normalize((ray.origin + t*ray.dir) - sphere[i].centre);
			color = vec4(1.0,0.2,0.1,1.0) * max(dot(-light_dir,N),0.0001);
			break;
		}		
	}
	
	imageStore(FinalImage,uv,color);
}