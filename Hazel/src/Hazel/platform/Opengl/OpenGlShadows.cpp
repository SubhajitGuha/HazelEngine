#include "hzpch.h"
#include "OpenGlShadows.h"
#include "glad/glad.h"

namespace Hazel {

	int OpenGlShadows::Cascade_level = 0;
	float OpenGlShadows::m_lamda = 0.2;
	OpenGlShadows::OpenGlShadows()
		:m_width(2048), m_height(2048)
	{
	}
	OpenGlShadows::OpenGlShadows(const float& width, const float& height)
		:m_width(width),m_height(height)
	{
		shadow_shader = Shader::Create("Assets/Shaders/ShadowShader.glsl");//texture shader
		m_LoadMesh = new LoadMesh("Assets/Meshes/Sphere.obj");//these should be the part of entity component system!!
		Cube = new LoadMesh("Assets/Meshes/Cube.obj");
		Plane = new LoadMesh("Assets/Meshes/Plane.obj");
		CreateShdowMap();
	}
	OpenGlShadows::~OpenGlShadows()
	{
	}
	void OpenGlShadows::RenderShadows(Scene& scene,const glm::vec3& LightPosition,ref<Shader> rendering_shader,EditorCamera& cam)
	{
		GLint OFb;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &OFb);
		auto size = RenderCommand::GetViewportSize();
		PrepareShadowProjectionMatrix(cam,LightPosition);

		std::vector<glm::mat4> LightProj_Matrices(MAX_CASCADES);
		for (int i = 0; i < MAX_CASCADES; i++)
			LightProj_Matrices[i] = m_ShadowProjection[i] * LightView[i];
		rendering_shader->SetMat4("MatrixShadow", LightProj_Matrices[0], MAX_CASCADES);
		unsigned int arr[] = { 11,12,13,14 };
		rendering_shader->SetIntArray("ShadowMap", MAX_CASCADES, arr);
		rendering_shader->SetFloatArray("Ranges", Ranges[0], MAX_CASCADES);

		for (int i = 0; i < MAX_CASCADES; i++) 
		{
			glm::mat4 LightProjection = m_ShadowProjection[i] * LightView[i];

			shadow_shader->Bind();
			shadow_shader->SetMat4("LightProjection", LightProjection);

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_id);
			glViewport(0, 0, m_width, m_height);
			//glClear(GL_DEPTH_BUFFER_BIT);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_id[i], 0);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
				HAZEL_CORE_INFO("shadow map FrameBuffer compleate!!");

			glClear(GL_DEPTH_BUFFER_BIT);
			scene.getRegistry().each([&](auto m_entity)//iterate through every entities and render them
				{
					//auto entt = item.second->GetEntity();//get the original entity (i.e. entt::entity returns an unsigned int)
					Entity Entity(&scene, m_entity);
					auto& transform = Entity.GetComponent<TransformComponent>().GetTransform();

					//if (camera.camera.bIsMainCamera) {
					if (Entity.HasComponent<SpriteRenderer>()) {
						auto SpriteRendererComponent = Entity.GetComponent<SpriteRenderer>();
						Renderer3D::DrawMesh(*m_LoadMesh, transform, SpriteRendererComponent.Color);
					}
					else
						Renderer3D::DrawMesh(*Cube, transform, Entity.m_DefaultColor);
				});
			//Renderer3D::DrawMesh(*Plane, { 0,0,0 }, { 10,10,10 }, { 0,0,0 });

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glViewport(0, 0, size.x, size.y);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glBindTextureUnit(i + 11, depth_id[i]);
		}
		
		
		//glBindTextureUnit(7, depth_id);
	}
	void OpenGlShadows::SetShadowMapResolution(const float& width, float height)
	{
		m_height = height;
		m_width = width;
	}
	void OpenGlShadows::CreateShdowMap()
	{
		glGenFramebuffers(1, &framebuffer_id);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);

		glGenTextures(MAX_CASCADES, depth_id);
		for (int i = 0; i < MAX_CASCADES; i++)
		{
			glBindTexture(GL_TEXTURE_2D, depth_id[i]);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
				HAZEL_CORE_INFO("shadow map FrameBuffer compleate!!");

			glBindTextureUnit(i+11, depth_id[i]);
		}
		m_Camera_Projection = glm::perspective(glm::radians(45.0f), 1.0f, 1.f, 1000.f);
	}
	void OpenGlShadows::PrepareShadowProjectionMatrix(EditorCamera& camera,const glm::vec3& LightPosition)
	{
		float NearPlane = 1.0f;
		float FarPlane = 100.0f;
		Ranges.resize(MAX_CASCADES+1);//send this in the fragment shader for determining which cascade to use
		Ranges[0] = NearPlane;
		Ranges[MAX_CASCADES] = FarPlane;
		for (int i = 1; i < MAX_CASCADES; i++)
		{
			// Practical Split Scheme: https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
			float uniform_split = ((FarPlane - NearPlane) / MAX_CASCADES) * (i);//uniform split
			float log_split = NearPlane * pow((FarPlane / NearPlane), i / (float)MAX_CASCADES);
			float practical = m_lamda * uniform_split + (1 - m_lamda) * log_split;
			Ranges[i] = practical;
		}
		for (int i = 1; i <= MAX_CASCADES; i++)
		{
			float m_NearPlane = Ranges[i - 1];
			float m_FarPlane = Ranges[i];

			m_Camera_Projection = glm::perspective(glm::radians(camera.GetVerticalFOV()), camera.GetAspectRatio(), m_NearPlane, m_FarPlane);
			
			glm::vec3 centre = glm::vec3(0.0f);
			glm::vec4 frustum_corners[8] = //cube coordinate in cannonical view volume
			{
			glm::vec4(-1.0f,  1.0f, -1.0f, 1.0f),
			glm::vec4(1.0f,  1.0f, -1.0f, 1.0f),
			glm::vec4(1.0f, -1.0f, -1.0f, 1.0f),
			glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f),
			glm::vec4(-1.0f,  1.0f,  1.0f, 1.0f),
			glm::vec4(1.0f,  1.0f,  1.0f, 1.0f),
			glm::vec4(1.0f, -1.0f,  1.0f, 1.0f),
			glm::vec4(-1.0f, -1.0f,  1.0f, 1.0f),
			};

			auto camera_view = camera.GetViewMatrix();
			glm::mat4 pv_inverse = glm::inverse(m_Camera_Projection * camera_view);
			for (int i = 0; i < 8; i++)
			{
				glm::vec4 p = pv_inverse * frustum_corners[i];//get the world space coordinate of frustum cube from cannonical-view-volume
				frustum_corners[i] = p/p.w;
				centre += glm::vec3(frustum_corners[i]);
			}
			centre /= 8.0f;
			HAZEL_INFO("centre {} {} {}",centre.x,centre.y,centre.z);

			LightView[i-1] = glm::lookAt(centre + glm::normalize(LightPosition), centre, { 0,1,0 });

			glm::mat4 matrix_lv = LightView[i - 1];
			float min_x = std::numeric_limits<float>::max();
			float max_x = std::numeric_limits<float>::lowest();
			float min_y = std::numeric_limits<float>::max();
			float max_y = std::numeric_limits<float>::lowest();
			float min_z = std::numeric_limits<float>::max();
			float max_z = std::numeric_limits<float>::lowest();

			for (int i = 0; i < 8; i++)
			{
				glm::vec4 corner = matrix_lv * frustum_corners[i];//convert to world coordinate
				corner /= corner.w;
				//frustum_corners[i] = corner; //convert from world to light space
				//frustum_corners[i] /= frustum_corners[i].w;

				min_x = std::min(min_x, corner.x);
				max_x = std::max(max_x, corner.x);
				min_y = std::min(min_y, corner.y);
				max_y = std::max(max_y, corner.y);
				min_z = std::min(min_z, corner.z);
				max_z = std::max(max_z, corner.z);
			}
			HAZEL_CORE_ERROR("minX,MaxX,MinY,MaxY,MinZ,MaxZ {} {} {} {} {} {}", min_x, max_x, min_y, max_y , min_z, max_z);
			int k = 5;

			constexpr float zMult = 100.0f;
			if (min_z < 0)
			{
				min_z *= zMult;
			}
			else
			{
				min_z /= zMult;
			}
			if (max_z < 0)
			{
				max_z /= zMult;
			}
			else
			{
				max_z *= zMult;
			}
			m_ShadowProjection.push_back(glm::ortho(min_x, max_x, min_y, max_y, min_z, max_z));
		}

	}
}