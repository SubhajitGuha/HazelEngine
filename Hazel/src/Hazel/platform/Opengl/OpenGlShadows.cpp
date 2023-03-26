#include "hzpch.h"
#include "OpenGlShadows.h"
#include "glad/glad.h"

namespace Hazel {
	int Shadows::Cascade_level = 0;
	float Shadows::m_lamda = 0.1;
	OpenGlShadows::OpenGlShadows()
		:m_width(4096), m_height(4096)
	{
	}
	OpenGlShadows::OpenGlShadows(const float& width, const float& height)
		:m_width(width),m_height(height)
	{
		shadow_shader = Shader::Create("Assets/Shaders/ShadowShader.glsl");//texture shader
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

		PrepareShadowProjectionMatrix(cam, LightPosition);//CREATE THE orthographic projection matrix

		std::vector<glm::mat4> LightProj_Matrices(MAX_CASCADES);
		for (int i = 0; i < MAX_CASCADES; i++)
			LightProj_Matrices[i] = m_ShadowProjection[i] * LightView[i];
		rendering_shader->SetMat4("MatrixShadow", LightProj_Matrices[0], MAX_CASCADES);
		unsigned int arr[] = { 11,12,13,14 };//these slots are explicitly used for all 4 seperate shadow maps
		rendering_shader->SetIntArray("ShadowMap", MAX_CASCADES, arr);
		rendering_shader->SetFloatArray("Ranges", Ranges[0], MAX_CASCADES);
		rendering_shader->SetMat4("view", cam.GetViewMatrix());//we need the camera's view matrix so that we can compute the distance comparison in view space

		shadow_shader->Bind();
		for (int i = 0; i < MAX_CASCADES; i++) 
			{
				glm::mat4 LightProjection = m_ShadowProjection[i] * LightView[i]; //placing a orthographic camera on the light position(i.e position at centroid of each frustum)

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
						auto mesh = Entity.GetComponent<StaticMeshComponent>();
						//if (camera.camera.bIsMainCamera) {
						if (Entity.HasComponent<SpriteRenderer>()) {
							auto SpriteRendererComponent = Entity.GetComponent<SpriteRenderer>();
							Renderer3D::DrawMesh(*mesh, transform, SpriteRendererComponent.Color);
						}
						else
							Renderer3D::DrawMesh(*mesh, transform, Entity.m_DefaultColor);
					});
				
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glViewport(0, 0, size.x, size.y);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
				HAZEL_CORE_INFO("shadow map FrameBuffer compleate!!");

			glBindTextureUnit(i+11, depth_id[i]);
		}
	}
	void OpenGlShadows::PrepareShadowProjectionMatrix(EditorCamera& camera,const glm::vec3& LightPosition)
	{
		m_ShadowProjection.clear();

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
		//iterate through all the cascade levels
		for (int i = 1; i <= MAX_CASCADES; i++)
		{
			float m_NearPlane = Ranges[i - 1];
			float m_FarPlane = Ranges[i];

			// create the view camera projection matrix based on the near and far plane
			m_Camera_Projection = glm::perspective(glm::radians(camera.GetVerticalFOV()), camera.GetAspectRatio(), m_NearPlane - 20.f, m_FarPlane );
			
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
			for (int j = 0; j < 8; j++)
			{
				glm::vec4 p = pv_inverse * frustum_corners[j];//get the world space coordinate of frustum cube from cannonical-view-volume
				frustum_corners[j] = p/p.w;
				centre += glm::vec3(frustum_corners[j]);
			}
			centre /= 8.0f;//calculate the centroid of the frustum cube and this will be the position for the light view matrix

			LightView[i-1] = glm::lookAt(centre - glm::normalize(LightPosition), centre, { 0.0,1.0,0.0 }); //move the camera to the centroid of each frustum

			glm::mat4 matrix_lv = LightView[i - 1];
			float min_x = std::numeric_limits<float>::max();
			float max_x = std::numeric_limits<float>::lowest();
			float min_y = std::numeric_limits<float>::max();
			float max_y = std::numeric_limits<float>::lowest();
			float min_z = std::numeric_limits<float>::max();
			float max_z = std::numeric_limits<float>::lowest();

			for (int j = 0; j < 8; j++)
			{
				glm::vec4 corner = matrix_lv * frustum_corners[j];//convert to light's coordinate
				corner /= corner.w;

				min_x = std::min(min_x, corner.x);
				max_x = std::max(max_x, corner.x);
				min_y = std::min(min_y, corner.y);
				max_y = std::max(max_y, corner.y);
				min_z = std::min(min_z, corner.z);
				max_z = std::max(max_z, corner.z);
			}
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