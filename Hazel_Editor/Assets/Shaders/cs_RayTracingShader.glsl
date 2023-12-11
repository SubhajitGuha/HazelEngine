#shader compute
#version 460 core

layout (local_size_x = 32, local_size_y = 32) in;

uniform layout(binding = 1, rgba8) writeonly image2D FinalImage;
uniform float viewport_w;
uniform float viewport_h;
uniform float focal_length;
uniform vec3 camera_pos;
uniform vec3 camera_viewdir;

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
bool rayHit(Sphere sh,Ray ray)
{
	vec3 ac = ray.origin - sh.centre;
	float a = dot(ray.dir,ray.dir);
	float b = 2*dot(ray.dir,ac);
	float c = dot(ac,ac)-pow(sh.radius,2);

	return (b*b - 4*a*c) >= 0;
}

void main()
{
	ivec2 uv = ivec2(gl_GlobalInvocationID.xy);
	vec4 color = vec4(0.1,0.9,0.4,1.0);

	vec2 image_res = gl_NumWorkGroups.xy*gl_WorkGroupSize.xy;
	vec3 up_vec = vec3(0,1,0);
	vec3 right_vec = normalize(cross(up_vec,camera_viewdir));
	vec3 v = cross(camera_viewdir,right_vec);

	vec3 viewport_u = viewport_w * right_vec; //define camera's x-coordinate
	vec3 viewport_v = -viewport_h * v; //define camera's y-coordinate

	vec3 del_u = viewport_u/image_res.x; //distance between 2 pixels in viewport represented as dir-vector
	vec3 del_v = viewport_v/image_res.y;

	vec3 viewport_upper_corner = camera_pos - camera_viewdir*focal_length - viewport_u/2.0 - viewport_v/2.0;
	vec3 pixel00_loc = viewport_upper_corner + 0.5*(del_u + del_v);

	vec3 pixel_pos = pixel00_loc + gl_GlobalInvocationID.x*del_u + gl_GlobalInvocationID.y*del_v;
	
	Ray ray;
	ray.origin = camera_pos;
	ray.dir = pixel_pos - camera_pos;

	Sphere sphere;
	sphere.centre = vec3(0,0,0);
	sphere.radius = 2.0;

	Sphere sphere1;
	sphere.centre = vec3(-5,0,0);
	sphere.radius = 5.0;

	if(rayHit(sphere,ray))
		color = vec4(1.0,0.3,0.0,1.0);
	
	imageStore(FinalImage,uv,color);
}