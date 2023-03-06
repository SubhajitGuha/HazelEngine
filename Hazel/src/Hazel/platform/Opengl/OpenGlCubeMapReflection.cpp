#include "hzpch.h"
#include "OpenGlCubeMapReflection.h"
#include "glad/glad.h"

namespace Hazel {
	LoadMesh* m_LoadMesh = nullptr;
	LoadMesh* Cube = nullptr;
	OpenGlCubeMapReflection::OpenGlCubeMapReflection()
		:cubemap_width(512),cubemap_height(512)
	{
		shader = Shader::Create("Assets/Shaders/ReflectionCubeMap.glsl");//texture shader
		m_LoadMesh = new LoadMesh("Assets/Meshes/Sphere.obj");
		Cube = new LoadMesh("Assets/Meshes/Cube.obj");

		CreateCubeMapTexture();
	}
	OpenGlCubeMapReflection::~OpenGlCubeMapReflection()
	{
	}
	void OpenGlCubeMapReflection::CreateCubeMapTexture()
	{
		glGenFramebuffers(1, &framebuffer_id);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);

		glGenTextures(1, &tex_id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);

		for (int i = 0; i < 6; i++)//iterate over 6 images each representing the side of a cube
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, cubemap_width, cubemap_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
			HAZEL_CORE_WARN("FrameBuffer compleate!!");

		GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, draw_buffers);

		glBindTextureUnit(10, tex_id);
	}

	auto SwitchToFace = [&](int index)
	{
		switch (index)
		{
			//pitch , yaw
		case 0:
			return glm::vec2(0, 90.0f);
		case 1:
			return glm::vec2(0, -90.0f);
		case 2:
			return glm::vec2(-90.0f, 180.0f);
		case 3:
			return glm::vec2(90.0f, 180.0f);
		case 4:
			return glm::vec2(0, 180.0f);
		case 5:
			return glm::vec2(0, 0);
		}
	};

	void OpenGlCubeMapReflection::RenderToCubeMap(Scene& scene)
	{
		shader->SetInt("env", slot);

		EditorCamera cam = EditorCamera(cubemap_width, cubemap_height);
		cam.SetPerspctive(90.00f, 0.01, 10000);
		cam.SetUPVector({ 0,-1,0 });
		cam.SetCameraPosition({ 0,-10,0 });

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_id);
		glViewport(0, 0, cubemap_width, cubemap_height);
		glm::vec3 pos = {0,-10,0};
		shader->SetFloat3("LightPosition", pos);
		for (int i = 0; i < 6; i++)
		{
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, tex_id, 0);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
				HAZEL_CORE_WARN("FrameBuffer compleate!!");
			//cam.OnUpdate(1);
			CubeMapEnvironment::RenderCubeMap(cam.GetViewMatrix(), cam.GetProjectionMatrix());
			glm::vec2 dir = SwitchToFace(i);
			cam.RotateCamera(dir.y, dir.x);

			shader->Bind();
			shader->SetMat4("u_ProjectionView", cam.GetProjectionView());
			shader->SetFloat3("EyePosition", cam.GetCameraPosition());

			scene.getRegistry().each([&](auto m_entity)
				{
					//auto entt = item.second->GetEntity();//get the original entity (i.e. entt::entity returns an unsigned int)
					Entity Entity(&scene, m_entity);
					auto& transform = Entity.GetComponent<TransformComponent>().GetTransform();
					glm::vec4 color;

					//if (camera.camera.bIsMainCamera) {
					if (Entity.HasComponent<SpriteRenderer>()) {
						auto SpriteRendererComponent = Entity.GetComponent<SpriteRenderer>();
						Renderer3D::DrawMesh(*m_LoadMesh, transform, SpriteRendererComponent.Color);
					}
					else
						Renderer3D::DrawMesh(*Cube, transform, Entity.m_DefaultColor);

				});
		}
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, 1920, 1080);
		glBindTextureUnit(10, tex_id);
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
}