#include "hzpch.h"
#include "OpenGlShadows.h"
#include "glad/glad.h"
#include "Hazel/Renderer/FoliageRenderer.h"
#include "Hazel/Renderer/Terrain.h"

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
		shadow_shader = Shader::Create("Assets/Shaders/ShadowShader.glsl");//shadow shader
		terrain_shadowShader = Shader::Create("Assets/Shaders/TerrainShadowShader.glsl");
		shadow_shaderInstanced = Shader::Create("Assets/Shaders/ShadowShaderInstanced.glsl");

		CreateShdowMap();
	}
	OpenGlShadows::~OpenGlShadows()
	{
	}
	void OpenGlShadows::RenderShadows(Scene& scene,const glm::vec3& LightPosition ,Camera& cam)
	{
		GLint OFb;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &OFb);
		auto size = RenderCommand::GetViewportSize();

		PrepareShadowProjectionMatrix(cam, LightPosition);//CREATE THE orthographic projection matrix

		shadow_shader->Bind();
		for (int i = 0; i < MAX_CASCADES; i++) 
			{
				glm::mat4 LightProjection = m_ShadowProjection[i] * LightView[i]; //placing a orthographic camera on the light position(i.e position at centroid of each frustum)

				shadow_shader->SetMat4("LightProjection", LightProjection);
				//Pass a alpha texture in the fragment shader to remove the depth values from the pixels that masked by alpha texture
				shadow_shader->SetInt("u_Alpha", ROUGHNESS_SLOT);//'2' is the slot for roughness map (alpha, roughness , AO in RGB) I have explicitely defined it for now

				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_id);
				glViewport(0, 0, m_width, m_height);
				//glClear(GL_DEPTH_BUFFER_BIT);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_id[i], 0);
				if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
					HAZEL_CORE_INFO("shadow map FrameBuffer compleate!!");

				glClear(GL_DEPTH_BUFFER_BIT);
				//glCullFace(GL_FRONT);
				scene.getRegistry().each([&](auto m_entity)//iterate through every entities and render them
					{
						Entity Entity(&scene, m_entity);
						if (Entity.GetComponent<StaticMeshComponent>().isFoliage == true)
							shadow_shader->SetInt("isFoliage", 1); //if the mesh is a foliage then in shader set it to true
						else
							shadow_shader->SetInt("isFoliage", 0);

						auto& transform = Entity.GetComponent<TransformComponent>().GetTransform();

						shadow_shader->SetMat4("u_Model", transform);

						auto mesh = Entity.GetComponent<StaticMeshComponent>();
						//if (camera.camera.bIsMainCamera) {
						if (Entity.HasComponent<SpriteRenderer>()) {
							auto SpriteRendererComponent = Entity.GetComponent<SpriteRenderer>();
							Renderer3D::DrawMesh(*mesh, transform, SpriteRendererComponent.Color);
						}
						else
							Renderer3D::DrawMesh(*mesh, transform, Entity.m_DefaultColor);
					});
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glViewport(0, 0, size.x, size.y);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}
	}
	void OpenGlShadows::RenderTerrainShadows(Scene& scene, const glm::vec3& LightPosition, Camera& cam)
	{
		GLint OFb;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &OFb);
		auto size = RenderCommand::GetViewportSize();

		PrepareShadowProjectionMatrix(cam, LightPosition);//CREATE THE orthographic projection matrix

		terrain_shadowShader->Bind();
		for (int i = 0; i < MAX_CASCADES; i++)
		{
			glm::mat4 LightProjection = m_ShadowProjection[i] * LightView[i]; //placing a orthographic camera on the light position(i.e position at centroid of each frustum)

			terrain_shadowShader->SetMat4("LightProjection", LightProjection);
			terrain_shadowShader->SetInt("u_HeightMap", HEIGHT_MAP_TEXTURE_SLOT);
			terrain_shadowShader->SetFloat("HEIGHT_SCALE", Terrain::HeightScale);
			terrain_shadowShader->SetMat4("u_Model", Terrain::m_terrainModelMat);
			terrain_shadowShader->SetMat4("u_View", cam.GetViewMatrix());

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_id);
			glViewport(0, 0, m_width, m_height);
			//glClear(GL_DEPTH_BUFFER_BIT);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_id[i], 0);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
				HAZEL_CORE_INFO("shadow map FrameBuffer compleate!!");

			glClear(GL_DEPTH_BUFFER_BIT);

			//render terrain
			//Needs change as I cannot just make terrain vertex array and terrain data public static		
			RenderCommand::DrawArrays(*Terrain::m_terrainVertexArray, Terrain::terrainData.size(), GL_PATCHES, 0);

			//scene entity shadow caster
			shadow_shader->Bind();
			shadow_shader->SetMat4("LightProjection", LightProjection);
			shadow_shader->SetInt("u_Alpha", ROUGHNESS_SLOT);//'2' is the slot for roughness map (alpha, roughness , AO in RGB) I have explicitely defined it for now

			scene.getRegistry().each([&](auto m_entity)//iterate through every entities and render them
				{
					Entity Entity(&scene, m_entity);
					if (Entity.GetComponent<StaticMeshComponent>().isFoliage == true)
						shadow_shader->SetInt("isFoliage", 1); //if the mesh is a foliage then in shader set it to true
					else
						shadow_shader->SetInt("isFoliage", 0);

					auto& transform = Entity.GetComponent<TransformComponent>().GetTransform();

					shadow_shader->SetMat4("u_Model", transform);

					auto mesh = Entity.GetComponent<StaticMeshComponent>();
					//if (camera.camera.bIsMainCamera) {
					if (Entity.HasComponent<SpriteRenderer>()) {
						auto SpriteRendererComponent = Entity.GetComponent<SpriteRenderer>();
						Renderer3D::DrawMesh(*mesh, transform, SpriteRendererComponent.Color);
					}
					else
						Renderer3D::DrawMesh(*mesh, transform, Entity.m_DefaultColor);
				});

			//for foliage instanced
			shadow_shaderInstanced->Bind();
			shadow_shaderInstanced->SetMat4("LightProjection", LightProjection);
			shadow_shaderInstanced->SetMat4("u_Model", Terrain::m_terrainModelMat);
			shadow_shaderInstanced->SetInt("u_Albedo", ALBEDO_SLOT); //alpha channel is being used
			for (Foliage* foliage : Foliage::foliageObjects)
			{
				if (foliage->bCanCastShadow) 
				{
					glDisable(GL_CULL_FACE);
					uint32_t bufferID = foliage->GetBufferID_LOD0();
					Renderer3D::InstancedFoliageData(*foliage->GetMesh(), bufferID);
					RenderCommand::DrawInstancedArrays(*foliage->GetMesh()->VertexArray, foliage->GetMesh()->Vertices.size(), foliage->NumLOD0);
					glEnable(GL_CULL_FACE);
					glCullFace(GL_BACK);
				}
			}
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glViewport(0, 0, size.x, size.y);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
	}
	void OpenGlShadows::RenderFoliageShadows(LoadMesh* mesh, uint32_t bufferID, int numMeshes, const glm::vec3& LightPosition, Camera& cam)
	{
		auto size = RenderCommand::GetViewportSize();
		PrepareShadowProjectionMatrix(cam, LightPosition);//CREATE THE orthographic projection matrix

		shadow_shaderInstanced->Bind();
		for (int i = 0; i < MAX_CASCADES; i++)
		{
			glm::mat4 LightProjection = m_ShadowProjection[i] * LightView[i]; //placing a orthographic camera on the light position(i.e position at centroid of each frustum)

			shadow_shaderInstanced->SetMat4("LightProjection", LightProjection);
			shadow_shaderInstanced->SetMat4("u_Model", Terrain::m_terrainModelMat);
			//shadow_shaderInstanced->SetMat4("u_View", cam.GetViewMatrix());

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_id);
			glViewport(0, 0, m_width, m_height);
			//glClear(GL_DEPTH_BUFFER_BIT);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_id[i], 0);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
				HAZEL_CORE_INFO("shadow map FrameBuffer compleate for foliage!!");

			glClear(GL_DEPTH_BUFFER_BIT);

			//render terrain
			//Needs change as I cannot just make terrain vertex array and terrain data public static		
			Renderer3D::InstancedFoliageData(*mesh, bufferID);

			//Renderer3D::BeginSceneFoliage(cam);
			//Renderer3D::DrawFoliageInstanced(*mesh, glm::mat4(1.0), numMeshes, { 1,1,1,1 }, Terrain::time);
			RenderCommand::DrawInstancedArrays(*mesh->VertexArray, mesh->Vertices.size(), numMeshes);

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glViewport(0, 0, size.x, size.y);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
	}
	void OpenGlShadows::SetShadowMapResolution(const float& width, float height)
	{
		m_height = height;
		m_width = width;
	}

	void OpenGlShadows::PassShadowUniforms(Camera& cam, ref<Shader> rendering_shader)
	{
		//this function passes the uniforms required for shadow rendering to the rendering shader
		rendering_shader->Bind();
		std::vector<glm::mat4> LightProj_Matrices(MAX_CASCADES);
		for (int i = 0; i < MAX_CASCADES; i++)
			LightProj_Matrices[i] = m_ShadowProjection[i] * LightView[i];
		rendering_shader->SetMat4("MatrixShadow", LightProj_Matrices[0], MAX_CASCADES);
		unsigned int arr[] = { SHDOW_MAP1,SHDOW_MAP2,SHDOW_MAP3,SHDOW_MAP4 };//these slots are explicitly used for all 4 seperate shadow maps
		rendering_shader->SetIntArray("ShadowMap", MAX_CASCADES, arr);
		rendering_shader->SetFloatArray("Ranges", Ranges[0], MAX_CASCADES);
		rendering_shader->SetMat4("view", cam.GetViewMatrix());//we need the camera's view matrix so that we can compute the distance comparison in view space
	}

	void OpenGlShadows::CreateShdowMap()
	{
		glGenFramebuffers(1, &framebuffer_id);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);

		glGenTextures(MAX_CASCADES, depth_id);
		for (int i = 0; i < MAX_CASCADES; i++)
		{
			glBindTexture(GL_TEXTURE_2D, depth_id[i]);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
				HAZEL_CORE_INFO("shadow map FrameBuffer compleate!!");
		}

		glBindTextureUnit(SHDOW_MAP1, depth_id[0]);
		glBindTextureUnit(SHDOW_MAP2, depth_id[1]);
		glBindTextureUnit(SHDOW_MAP3, depth_id[2]);
		glBindTextureUnit(SHDOW_MAP4, depth_id[3]);
	}
	void OpenGlShadows::PrepareShadowProjectionMatrix(Camera& camera,const glm::vec3& LightPosition)
	{
		m_ShadowProjection.clear();

		float NearPlane = 1.0f;
		float FarPlane = 800.0f;
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
			m_Camera_Projection = glm::perspective(glm::radians(camera.GetVerticalFOV()), camera.GetAspectRatio(), NearPlane, m_FarPlane );
			
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
			glm::mat4 pv_inverse = glm::inverse(camera_view) * glm::inverse(m_Camera_Projection);
			for (int j = 0; j < 8; j++)
			{
				glm::vec4 p = pv_inverse * frustum_corners[j];//get the world space coordinate of frustum cube from cannonical-view-volume
				frustum_corners[j] = p/p.w;
				centre += glm::vec3(frustum_corners[j]);
			}
			centre /= 8.0f;//calculate the centroid of the frustum cube and this will be the position for the light view matrix

			LightView[i-1] = glm::lookAt(centre , centre + glm::normalize(LightPosition), { 0.0,1.0,0.0 }); //move the camera to the centroid of each frustum

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
