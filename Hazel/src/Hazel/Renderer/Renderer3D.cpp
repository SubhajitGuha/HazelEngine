#include "hzpch.h"
#include "Renderer3D.h"
#include "RenderCommand.h"
#include "Buffer.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "stb_image.h"
#include "glad/glad.h"
#include "Hazel/Renderer/Shadows.h"
#include "Hazel/Scene/PointLight.h"
#include "Hazel/platform/Opengl/OpenGlSSAO.h"//temporary testing purpose
#include "Hazel/Renderer/Terrain.h"
#include "Hazel/Renderer/SkyRenderer.h"
#include "Hazel/Renderer/DeferredRenderer.h"
#include "Hazel/Renderer/Antialiasing.h"
#include "Material.h"
#include "Hazel/ResourceManager.h"


namespace Hazel {
	//Camera* m_camera;
	GLsync syncObj;
	glm::mat4 Renderer3D::m_oldProjectionView = glm::mat4(1.0f);
	glm::vec3 Renderer3D::m_SunLightDir = { 0,-5,0.60 };//initial light position
	glm::vec3 Renderer3D::m_oldSunLightDir = {0,0,0};
	glm::vec3 Renderer3D::m_SunColor = { 1,1,1 };
	float Renderer3D::m_SunIntensity = 1.0f;

	unsigned int Renderer3D::depth_id[4];
	int Renderer3D::index = 0;
	unsigned int Renderer3D::ssao_id = 0;
	struct VertexAttributes {
		//glm::vec3 Position;
		glm::vec4 Position;
		glm::vec2 TextureCoordinate;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec3 BiNormal;
		unsigned int Material_index = 0;//serves as an index to the array of texture slot which is passed as an uniform in init()
		VertexAttributes(const glm::vec4& Position, const glm::vec2& TextureCoordinate, const glm::vec3& normal = { 0,0,0 }, const glm::vec3& Tangent = { 0,0,0 }, const glm::vec3& BiNormal = {0,0,0}, unsigned int Material_index = 0)
		{
			this->Position = Position;
			this->TextureCoordinate = TextureCoordinate;
			this->Material_index = Material_index;
			Normal = normal;
			this->Tangent = Tangent;
			this->BiNormal = BiNormal;
		}
		//may more ..uv coord , tangents , normals..
	};

	struct Renderer3DStorage {		
		ref<OpenGlSSAO> ssao;
		ref<Antialiasing> taa; //temporal antialiasing
		ref<Shadows> shadow_map;
		ref<CubeMapReflection> reflection;
		ref<Shader> shader, foliage_shader, foliageShader_instanced;
	};
	static Renderer3DStorage* m_data;

	void Renderer3D::Init(int width, int height)
	{
		m_data = new Renderer3DStorage;
		DefferedRenderer::Init(width,height);//Initilize the Deferred Renderer

		m_data->shader = (Shader::Create("Assets/Shaders/3D_2_In_1Shader.glsl"));//texture shader
		m_data->shader->SetInt("SSAO", SSAO_BLUR_SLOT);
		m_data->foliage_shader = Shader::Create("Assets/Shaders/FoliageShader.glsl");//foliage shader
		m_data->foliage_shader->SetInt("SSAO", SSAO_BLUR_SLOT);
		m_data->foliageShader_instanced = Shader::Create("Assets/Shaders/FoliageShader_Instanced.glsl");//this is not ideal!! I just cannot handle shaders like this in a long run
		m_data->foliageShader_instanced->SetInt("SSAO", SSAO_BLUR_SLOT);

		DefferedRenderer::GetDeferredPassShader()->Bind();
		DefferedRenderer::GetDeferredPassShader()->SetInt("SSAO", SSAO_BLUR_SLOT);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//Loading cube map so that it can act as an environment light
		//m_data->reflection = CubeMapReflection::Create();
		m_data->taa = Antialiasing::Create(width,height);
		m_data->ssao = std::make_shared<OpenGlSSAO>(width/2,height/2);
		m_data->shadow_map = Shadows::Create(2048, 2048);//create a 2048x2048 shadow map
		for (int i = 0; i < 4; i++)
			depth_id[i] = m_data->shadow_map->GetDepth_ID(i);
		ssao_id = m_data->ssao->GetSSAOid();
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		SetSunLightDirection({ 3,5,2});
	}

	void Renderer3D::BeginScene(OrthographicCamera& camera)
	{
		m_data->shader->Bind();//bind the textureShader
		m_data->shader->SetMat4("u_ProjectionView", camera.GetProjectionViewMatix());
		m_data->shader->SetFloat3("EyePosition", camera.GetPosition());
	}

	void Renderer3D::BeginScene(Camera& camera,const ref<Shader>& otherShader)
	{
		if (otherShader == nullptr) {
			m_data->shader->Bind();//bind the textureShader
			m_data->shader->SetMat4("u_ProjectionView", camera.GetProjectionView());//here the projection is ProjectionView
			//m_data->shader->
			m_data->shader->SetFloat3("EyePosition", camera.GetCameraPosition());//get the eye position for specular lighting calculation
		}
		else
		{
			otherShader->Bind();//bind the textureShader
			otherShader->SetMat4("u_ProjectionView", camera.GetProjectionView());//here the projection is ProjectionView
			otherShader->SetMat4("u_View", camera.GetViewMatrix());
			otherShader->SetMat4("u_oldProjectionView", m_oldProjectionView);
			//otherShader->SetFloat3("EyePosition", camera.GetCameraPosition());//get the eye position for specular lighting calculation
		}
	}

	void Renderer3D::BeginSceneFoliage(Camera& camera, const ref<Shader>& otherShader)
	{
		m_data->foliage_shader->Bind();//bind the textureShader
		m_data->foliage_shader->SetMat4("u_ProjectionView", camera.GetProjectionView());//here the projection is ProjectionView
		m_data->foliage_shader->SetFloat3("EyePosition", camera.GetCameraPosition());//get the eye position for specular lighting calculation

		if (otherShader == nullptr) 
		{
			m_data->foliageShader_instanced->Bind();//bind the textureShader
			m_data->foliageShader_instanced->SetMat4("u_ProjectionView", camera.GetProjectionView());//here the projection is ProjectionView
			m_data->foliageShader_instanced->SetMat4("u_Projection", camera.GetProjectionMatrix());
			m_data->foliageShader_instanced->SetMat4("u_View", camera.GetViewMatrix());
			m_data->foliageShader_instanced->SetMat4("u_oldProjectionView", m_oldProjectionView);
			m_data->foliageShader_instanced->SetFloat3("u_cameraPos", camera.GetCameraPosition());
			m_data->foliageShader_instanced->SetFloat3("EyePosition", camera.GetCameraPosition());//get the eye position for specular lighting calculation

		}
		else
		{
			otherShader->Bind();//bind the textureShader
			otherShader->SetMat4("u_ProjectionView", camera.GetProjectionView());//here the projection is ProjectionView
			otherShader->SetMat4("u_Projection", camera.GetProjectionMatrix());
			otherShader->SetMat4("u_View", camera.GetViewMatrix());
			otherShader->SetMat4("u_oldProjectionView", m_oldProjectionView);
			otherShader->SetFloat3("u_cameraPos", camera.GetCameraPosition());
			otherShader->SetFloat3("EyePosition", camera.GetCameraPosition());//get the eye position for specular lighting calculation
		}
	}

	void Renderer3D::EndScene()
	{
	}

	void Renderer3D::SetSunLightDirection(const glm::vec3& pos)
	{
		m_data->shader->Bind();
		m_data->shader->SetFloat3("DirectionalLight_Direction", pos);

		m_data->foliage_shader->Bind();
		m_data->foliage_shader->SetFloat3("DirectionalLight_Direction", pos);

		m_data->foliageShader_instanced->Bind(); //this is not ideal!! I just cannot handle shaders like this in a long run
		m_data->foliageShader_instanced->SetFloat3("DirectionalLight_Direction", pos);

		DefferedRenderer::GetDeferredPassShader()->Bind();
		DefferedRenderer::GetDeferredPassShader()->SetFloat3("DirectionalLight_Direction", pos);

	}

	void Renderer3D::SetSunLightColorAndIntensity(const glm::vec3& color, float Intensity)
	{
		m_data->shader->Bind();
		m_data->shader->SetFloat3("SunLight_Color", color);
		m_data->shader->SetFloat("SunLight_Intensity", Intensity);

		//do foliage shader
		m_data->foliage_shader->Bind();
		m_data->foliage_shader->SetFloat3("SunLight_Color", color);
		m_data->foliage_shader->SetFloat("SunLight_Intensity", Intensity);

		//do foliage instanced shader
		m_data->foliageShader_instanced->Bind();
		m_data->foliageShader_instanced->SetFloat3("SunLight_Color", color);
		m_data->foliageShader_instanced->SetFloat("SunLight_Intensity", Intensity);

		DefferedRenderer::GetDeferredPassShader()->Bind();
		DefferedRenderer::GetDeferredPassShader()->SetFloat3("SunLight_Color", color);
		DefferedRenderer::GetDeferredPassShader()->SetFloat("SunLight_Intensity", Intensity);
	}

	void Renderer3D::SetPointLightPosition(const std::vector<PointLight*>& Lights)
	{
		std::vector<glm::vec3> pos(Lights.size());
		std::vector<glm::vec3> col(Lights.size());
		for (int i=0 ;i<Lights.size();i++)
		{
			pos[i] = Lights[i]->GetLightPosition();
		}
		for (int i = 0; i < Lights.size(); i++)
		{
			col[i] = Lights[i]->GetLightColor();
		}

		m_data->shader->Bind();
		m_data->shader->SetFloat3Array("PointLight_Position", &pos[0].x,Lights.size());
		m_data->shader->SetFloat3Array("PointLight_Color", &col[0].x, Lights.size());
		m_data->shader->SetInt("Num_PointLights", pos.size());

		m_data->foliage_shader->Bind();
		m_data->foliage_shader->SetFloat3Array("PointLight_Position", &pos[0].x, Lights.size());
		m_data->foliage_shader->SetFloat3Array("PointLight_Color", &col[0].x, Lights.size());
		m_data->foliage_shader->SetInt("Num_PointLights", pos.size());

		m_data->foliageShader_instanced->Bind();
		m_data->foliageShader_instanced->SetFloat3Array("PointLight_Position", &pos[0].x, Lights.size());
		m_data->foliageShader_instanced->SetFloat3Array("PointLight_Color", &col[0].x, Lights.size());
		m_data->foliageShader_instanced->SetInt("Num_PointLights", pos.size());

		DefferedRenderer::GetDeferredPassShader()->Bind();
		DefferedRenderer::GetDeferredPassShader()->SetFloat3Array("PointLight_Position", &pos[0].x, Lights.size());
		DefferedRenderer::GetDeferredPassShader()->SetFloat3Array("PointLight_Color", &col[0].x, Lights.size());
		DefferedRenderer::GetDeferredPassShader()->SetInt("Num_PointLights", pos.size());
	}

	void Renderer3D::DrawMesh(LoadMesh& mesh,glm::mat4& transform, const glm::vec4& color, const float& material_Roughness , const float& material_metallic, ref<Shader> otherShader)
	{	
		for (auto& sub_mesh : mesh.m_subMeshes) 
		{
			ref<Material> material = ResourceManager::allMaterials[sub_mesh.m_MaterialID];

			if (!material) {
				HAZEL_CORE_ERROR("Material dosent exist");
				return; //dont render in case of non existing material
			}
			material->Diffuse_Texture->Bind(ALBEDO_SLOT);
			material->Roughness_Texture->Bind(ROUGHNESS_SLOT);
			material->Normal_Texture->Bind(NORMAL_SLOT);

			if (!otherShader) //shader will be defined from material
			{
				m_data->shader->SetFloat("Roughness", material->GetRoughness()); //send the roughness value
				m_data->shader->SetFloat("Metallic", material->GetMetalness()); //send the metallic value
				m_data->shader->SetInt("u_Albedo", ALBEDO_SLOT);//bind albedo texture array to slot1;
				m_data->shader->SetInt("u_Roughness", ROUGHNESS_SLOT);
				m_data->shader->SetInt("u_NormalMap", NORMAL_SLOT);
				m_data->shader->SetMat4("u_Model", transform);
				m_data->shader->SetFloat4("m_color", material->GetColor());
			}
			else
			{
				otherShader->SetFloat("Roughness", material->GetRoughness()); //send the roughness value
				otherShader->SetFloat("Metallic", material->GetMetalness()); //send the metallic value
				otherShader->SetInt("u_Albedo", ALBEDO_SLOT);//bind albedo texture array to slot1;
				otherShader->SetInt("u_Roughness", ROUGHNESS_SLOT);
				otherShader->SetInt("u_NormalMap", NORMAL_SLOT);
				otherShader->SetMat4("u_Model", transform);
				otherShader->SetFloat4("m_color", material->GetColor());			
			}

			RenderCommand::DrawArrays(*sub_mesh.VertexArray, sub_mesh.numVertices);
		}
	}

	void Renderer3D::DrawFoliageInstanced(SubMesh& sub_mesh, glm::mat4& transform, uint32_t& indirectBufferID, float TimeElapsed, bool applyGradientMask, bool enableWind)
	{
		m_data->foliageShader_instanced->Bind();
		glDisable(GL_CULL_FACE);
		ref<Material> material = ResourceManager::allMaterials[sub_mesh.m_MaterialID]; //get material from the resource manager
		
		if (!material) {
			HAZEL_CORE_ERROR("Material dosent exist");
			return; //dont render in case of non existing material
		}
		m_data->foliageShader_instanced->SetFloat("Roughness", material->GetRoughness()); //send the roughness value
		m_data->foliageShader_instanced->SetFloat("Metallic", material->GetMetalness()); //send the metallic value
		
		material->Diffuse_Texture->Bind(ALBEDO_SLOT);
		material->Roughness_Texture->Bind(ROUGHNESS_SLOT);
		material->Normal_Texture->Bind(NORMAL_SLOT);
		
		m_data->foliageShader_instanced->SetInt("u_Albedo", ALBEDO_SLOT);//bind albedo texture to slot1;
		m_data->foliageShader_instanced->SetInt("u_Roughness", ROUGHNESS_SLOT);
		m_data->foliageShader_instanced->SetInt("u_NormalMap", NORMAL_SLOT);
		m_data->foliageShader_instanced->SetMat4("u_Model", transform);
		m_data->foliageShader_instanced->SetFloat4("m_color", material->GetColor());
		m_data->foliageShader_instanced->SetFloat3("u_BoundsExtent", (sub_mesh.mesh_bounds.aabbMax-sub_mesh.mesh_bounds.aabbMin));
		m_data->foliageShader_instanced->SetFloat("u_Time", TimeElapsed);
		m_data->foliageShader_instanced->SetInt("Noise", PERLIN_NOISE_TEXTURE_SLOT);
		m_data->foliageShader_instanced->SetInt("applyGradientMask", applyGradientMask);
		m_data->foliageShader_instanced->SetInt("enableWind", enableWind);

		RenderCommand::DrawArraysIndirect(*sub_mesh.VertexArray, indirectBufferID);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);		
	}

	void Renderer3D::InstancedFoliageData(LoadMesh& mesh, uint32_t& bufferIndex)
	{
		//needs to be refactored!!
		for (auto& sub_mesh : mesh.m_subMeshes)
		{
			glBindBuffer(GL_ARRAY_BUFFER, bufferIndex);

			sub_mesh.VertexArray->Bind();
			glEnableVertexAttribArray(5);
			glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)0);
			glEnableVertexAttribArray(6);
			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(1 * sizeof(glm::vec4)));
			glEnableVertexAttribArray(7);
			glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(2 * sizeof(glm::vec4)));
			glEnableVertexAttribArray(8);
			glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(3 * sizeof(glm::vec4)));

			glVertexAttribDivisor(5, 1);//repeat 1ce per instance
			glVertexAttribDivisor(6, 1);
			glVertexAttribDivisor(7, 1);
			glVertexAttribDivisor(8, 1);
		}
	}
	
	void Renderer3D::AllocateInstancedFoliageData(LoadMesh& mesh,const size_t& size, uint32_t& bufferIndex)
	{
		//glGenBuffers(1, &bufferIndex);
		//glBindBuffer(GL_ARRAY_BUFFER, bufferIndex);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * size, nullptr, GL_DYNAMIC_DRAW);
		for (auto& sub_mesh : mesh.m_subMeshes)
		{
			sub_mesh.VertexArray->Bind();
			glEnableVertexAttribArray(5);
			glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)0);
			glEnableVertexAttribArray(6);
			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(1 * sizeof(glm::vec4)));
			glEnableVertexAttribArray(7);
			glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(2 * sizeof(glm::vec4)));
			glEnableVertexAttribArray(8);
			glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(3 * sizeof(glm::vec4)));

			glVertexAttribDivisor(5, 1);//repeat 1ce per instance
			glVertexAttribDivisor(6, 1);
			glVertexAttribDivisor(7, 1);
			glVertexAttribDivisor(8, 1);
		}
	}
	void Renderer3D::SetUpCubeMapReflections(Scene& scene)
	{
		//m_data->reflection->RenderToCubeMap(scene);
		m_data->shader->Bind();//you need to bind this other wise nothing will be rendererd
		m_data->shader->SetInt("diffuse_env", IRR_ENV_SLOT);//for now assign to 10 :)
		m_data->shader->SetInt("specular_env", ENV_SLOT);//for now assign to 18 :)

		m_data->foliage_shader->Bind();//you need to bind this other wise nothing will be rendererd
		m_data->foliage_shader->SetInt("diffuse_env", IRR_ENV_SLOT);//for now assign to 10 :)
		m_data->foliage_shader->SetInt("specular_env", ENV_SLOT);//for now assign to 18 :)

		m_data->foliageShader_instanced->Bind();
		m_data->foliageShader_instanced->SetInt("diffuse_env", IRR_ENV_SLOT);
		m_data->foliageShader_instanced->SetInt("specular_env", ENV_SLOT);

		DefferedRenderer::GetDeferredPassShader()->Bind();
		DefferedRenderer::GetDeferredPassShader()->SetInt("diffuse_env", IRR_ENV_SLOT);
		DefferedRenderer::GetDeferredPassShader()->SetInt("specular_env", ENV_SLOT);
		DefferedRenderer::GetDeferredPassShader()->SetInt("BRDF_LUT", LUT_SLOT);

	}

	void Renderer3D::RenderShadows(Scene& scene, Camera& camera)
	{
		m_data->shadow_map->RenderTerrainShadows(scene, m_SunLightDir, camera);
		m_data->shadow_map->PassShadowUniforms(camera, m_data->shader);
		m_data->shadow_map->PassShadowUniforms(camera, m_data->foliage_shader);
		m_data->shadow_map->PassShadowUniforms(camera, m_data->foliageShader_instanced);
		m_data->shadow_map->PassShadowUniforms(camera, scene.m_Terrain->m_terrainShader);
		m_data->shadow_map->PassShadowUniforms(camera, DefferedRenderer::GetDeferredPassShader());
	}

	void Renderer3D::AmbiantOcclusion(Scene& scene, Camera& camera)
	{
		m_data->ssao->CaptureScene(scene, camera);
		//m_data->shader->Bind();
	}

	ref<Shadows>& Renderer3D::GetShadowObj()
	{
		return m_data->shadow_map;
	}

	void Renderer3D::SetTransperancy(float val)
	{
		m_data->shader->Bind();//you need to bind this other wise nothing will be rendererd
		m_data->shader->SetFloat("Transperancy", val);//for now assign to 10 :)
	}

	ref<Shader>& Renderer3D::GetFoliageInstancedShader()
	{
		return m_data->foliageShader_instanced;
	}
	void Renderer3D::RenderWithAntialiasing()
	{
		m_data->taa->Update();
	}
	void Renderer3D::RenderScene_Deferred(Scene* scene)
	{
		SkyRenderer::RenderSky(*scene->GetCamera());
		DefferedRenderer::DeferredRenderPass();
		m_oldProjectionView = scene->GetCamera()->GetProjectionView(); //update the old projection-view matrix after every thing is rendered
	}

	void Renderer3D::ForwardRenderPass(Scene* scene)
	{
		DefferedRenderer::GenerateGBuffers(scene);	
		RenderShadows(*scene, *scene->GetCamera());
		AmbiantOcclusion(*scene, *scene->GetCamera());
	}

}
