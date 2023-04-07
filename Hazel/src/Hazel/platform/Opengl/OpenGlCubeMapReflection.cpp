#include "hzpch.h"
#include "OpenGlCubeMapReflection.h"
#include "glad/glad.h"
#include "stb_image.h"
#include "stb_image_resize.h"

namespace Hazel {
	LoadMesh* m_LoadMesh = nullptr;
	LoadMesh* Cube = nullptr,*Plane=nullptr;
	OpenGlCubeMapReflection::OpenGlCubeMapReflection()
		:cubemap_width(2048),cubemap_height(2048)
	{
		RenderCommand::Init();
		shader = Shader::Create("Assets/Shaders/ReflectionCubeMap.glsl");//texture shader
		m_LoadMesh = new LoadMesh("Assets/Meshes/Sphere.obj");
		Cube = new LoadMesh("Assets/Meshes/Cube.obj");
		Plane = new LoadMesh("Assets/Meshes/Plane.obj");
		CreateCubeMapTexture();
	}
	OpenGlCubeMapReflection::~OpenGlCubeMapReflection()
	{
	}
	void OpenGlCubeMapReflection::CreateCubeMapTexture()
	{
		float* data = nullptr;
		int width, height, channels;

		glGenTextures(1, &tex_id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);

		glGenRenderbuffers(1, &depth_id);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_id);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, cubemap_width, cubemap_height);

		std::string filename = "Assets/Textures/Outdoor-Cube-map/";

		for (int i = 0; i < 6; i++)//iterate over 6 images each representing the side of a cube
		{
			stbi_set_flip_vertically_on_load_thread(1);
			data = stbi_loadf((filename + "irr_" + std::to_string(i) + ".jpg").c_str(), &width, &height, &channels, 0);
			if (!data)
				HAZEL_CORE_ERROR("Irradiance map not loaded");
			else
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
		}

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		HAZEL_CORE_WARN(glGetError());

		glBindTextureUnit(10, tex_id);
	}

	void OpenGlCubeMapReflection::RenderToCubeMap(Scene& scene)
	{
		//shader->Bind();
		//auto size = RenderCommand::GetViewportSize();
		//
		//EditorCamera cam = EditorCamera();
		//cam.SetPerspctive(90.00f, -10.0f, 1000.0f);//*** mann this game me a headache (by not converting the FOV to radians) the fov is converted to radians in the editorCamera class :)
		//cam.SetUPVector({ 0,-1,0 });
		//cam.SetCameraPosition({ 0,-5,0 });
		//
		////glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_id);
		//glViewport(0, 0, cubemap_width , cubemap_height);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//std::vector<glm::vec3> dir = { {1,0,0},{-1,0,0},{0,-1,0},{0,1,0},{0,0,1},{0,0,-1} };
		//glm::vec3 pos = {2,3,6};
		////shader->SetFloat3("LightPosition", pos);//world position
		//shader->SetInt("env", 18);//18th slot is the slot that stores cubemap texture
		//for (int i = 0; i < 6; i++)
		//{
		//	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i , tex_id, 0);//Render the scene in the corresponding face on the cube map and "GL_COLOR_ATTACHMENT0" Represents the Render target where the fragment shader should output the color
		//	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		//		HAZEL_CORE_WARN("FrameBuffer compleate!!");
		//
		//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//clear the buffers each time
		//	//HAZEL_CORE_ERROR(glGetError());
		//
		//	SwitchToFace(i);//rotate the camera
		//	cam.RotateCamera(yaw, pitch);
		//
		//	//Render the cube map
		//	//shader->Bind();
		//	CubeMapEnvironment::RenderQuad(cam.GetViewMatrix(), cam.GetProjectionMatrix());//cubemap shader is binded here follow this order
		//
		//	/*shader->Bind();//bind the shader and pass the projectionview and Eye pos as uniform.
		//	shader->SetMat4("u_ProjectionView", cam.GetProjectionView());
		//	shader->SetFloat3("EyePosition", cam.GetCameraPosition());
		//	scene.getRegistry().each([&](auto m_entity)//iterate through every entities and render them
		//		{
		//			//auto entt = item.second->GetEntity();//get the original entity (i.e. entt::entity returns an unsigned int)
		//			Entity Entity(&scene, m_entity);
		//			auto& transform = Entity.GetComponent<TransformComponent>().GetTransform();
		//			glm::vec4 color;
		//
		//			//if (camera.camera.bIsMainCamera) {
		//			if (Entity.HasComponent<SpriteRenderer>()) {
		//				auto SpriteRendererComponent = Entity.GetComponent<SpriteRenderer>();
		//				Renderer3D::DrawMesh(*m_LoadMesh, transform, SpriteRendererComponent.Color);
		//			}
		//			else
		//				Renderer3D::DrawMesh(*Cube, transform, Entity.m_DefaultColor);
		//
		//		});
		//			Renderer3D::DrawMesh(*Plane, { 0,0,0 }, { 100,100,100 }, { 0,0,0 });*/
		//}
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);//unbind the framebuffer to compleate the capturing process
		//glViewport(0, 0, size.x, size.y);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glBindTextureUnit(10, tex_id);
	}

	void OpenGlCubeMapReflection::Bind(int slot)
	{
		glBindTextureUnit(slot, tex_id);
	}

	void OpenGlCubeMapReflection::UnBind()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	unsigned int OpenGlCubeMapReflection::GetTexture_ID()
	{
		return tex_id;
	}

	void OpenGlCubeMapReflection::SetCubeMapResolution(float width, float height)
	{
		cubemap_width = width;
		cubemap_height = height;
	}
	void  OpenGlCubeMapReflection::SwitchToFace(int n)
	{
		switch (n)
		{
			//pitch , yaw
		case 0:
			pitch = 0;
			yaw = -90;
			break;
		case 1:
			pitch = 0;
			yaw = 90;
			break;
		case 2:
			pitch = 90.0f;
			yaw = 180;
			break;
		case 3:
			pitch = -90;
			yaw = 180;
			break;
		case 4:
			pitch = 0;
			yaw = 0;
			break;
		case 5:
			pitch = 0;
			yaw = 180;
			break;
		}
	}
}