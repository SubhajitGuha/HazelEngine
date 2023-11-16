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
#include "Material.h"

namespace Hazel {
	//Camera* m_camera;
	GLsync syncObj;
	glm::vec3 Renderer3D::m_SunLightDir = { 0,1,0.60 };//initial light position
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
		int max_Vertices = 2000000;
		VertexAttributes* Vertex=nullptr;
		uint32_t vertexb_id;
		ref<OpenGlSSAO> ssao;
		ref<Shadows> shadow_map;
		ref<CubeMapReflection> reflection;
		ref<Shader> shader, foliage_shader, foliageShader_instanced;
		ref<Texture2D> WhiteTex,tex;
		ref<BufferLayout> bl;
		ref<VertexBuffer> vb;
		ref<IndexBuffer> ib;
		ref<VertexArray> va;

		uint32_t m_VertexCounter = 0;
	};
	static Renderer3DStorage* m_data;

	void Renderer3D::Init()
	{
		m_data = new Renderer3DStorage;
		DefferedRenderer::Init(1920,1080);//Initilize the Deferred Renderer

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
		m_data->ssao = std::make_shared<OpenGlSSAO>(1024,1024);
		m_data->shadow_map = Shadows::Create(4096/2, 4096/2);//create a 2048x2048 shadow map
		for (int i = 0; i < 4; i++)
			depth_id[i] = m_data->shadow_map->GetDepth_ID(i);
		ssao_id = m_data->ssao->GetSSAOid();
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		m_data->WhiteTex = Texture2D::Create(1, 1, 0xffffffff);//create a default white texture
		m_data->tex = Texture2D::Create("Assets/Textures/Test.png");
		m_data->tex->Bind(1);
		m_data->WhiteTex->Bind(0);
		
		SetSunLightDirection({ 3,5,2});
	}

	void Renderer3D::BeginScene(OrthographicCamera& camera)
	{
		m_data->shader->Bind();//bind the textureShader
		m_data->shader->SetMat4("u_ProjectionView", camera.GetProjectionViewMatix());
		m_data->shader->SetFloat3("EyePosition", camera.GetPosition());
	}

	void Renderer3D::BeginScene(Camera& camera)
	{
		//Init();
		m_data->shader->Bind();//bind the textureShader
		m_data->shader->SetMat4("u_ProjectionView", camera.GetProjectionView());//here the projection is ProjectionView
		m_data->shader->SetFloat3("EyePosition", camera.GetCameraPosition());//get the eye position for specular lighting calculation
		//m_camera = camera;
		m_data->m_VertexCounter = 0;
		//Renderer2D::LineBeginScene(camera);
	}

	void Renderer3D::BeginSceneFoliage(Camera& camera)
	{
		m_data->foliage_shader->Bind();//bind the textureShader
		m_data->foliage_shader->SetMat4("u_ProjectionView", camera.GetProjectionView());//here the projection is ProjectionView
		m_data->foliage_shader->SetFloat3("EyePosition", camera.GetCameraPosition());//get the eye position for specular lighting calculation

		m_data->foliageShader_instanced->Bind();//bind the textureShader
		m_data->foliageShader_instanced->SetMat4("u_ProjectionView", camera.GetProjectionView());//here the projection is ProjectionView
		m_data->foliageShader_instanced->SetMat4("u_Projection", camera.GetProjectionMatrix());
		m_data->foliageShader_instanced->SetMat4("u_View", camera.GetViewMatrix());
		m_data->foliageShader_instanced->SetFloat3("u_cameraPos", camera.GetCameraPosition());
		m_data->foliageShader_instanced->SetFloat3("EyePosition", camera.GetCameraPosition());//get the eye position for specular lighting calculation
	}

	void Renderer3D::EndScene()
	{
	}

	void Renderer3D::SetSunLightDirection(const glm::vec3& pos)
	{
		m_SunLightDir = pos;
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
			sub_mesh.m_Material->Diffuse_Texture->Bind(ALBEDO_SLOT);
			sub_mesh.m_Material->Roughness_Texture->Bind(ROUGHNESS_SLOT);
			sub_mesh.m_Material->Normal_Texture->Bind(NORMAL_SLOT);

			if (!otherShader) {
				m_data->shader->SetFloat("Roughness", material_Roughness); //send the roughness value
				m_data->shader->SetFloat("Metallic", material_metallic); //send the metallic value
				m_data->shader->SetInt("u_Albedo", ALBEDO_SLOT);//bind albedo texture array to slot1;
				m_data->shader->SetInt("u_Roughness", ROUGHNESS_SLOT);
				m_data->shader->SetInt("u_NormalMap", NORMAL_SLOT);
				m_data->shader->SetMat4("u_Model", transform);
				m_data->shader->SetFloat4("m_color", color);
			}
			else
			{
				otherShader->SetFloat("Roughness", material_Roughness); //send the roughness value
				otherShader->SetFloat("Metallic", material_metallic); //send the metallic value
				otherShader->SetInt("u_Albedo", ALBEDO_SLOT);//bind albedo texture array to slot1;
				otherShader->SetInt("u_Roughness", ROUGHNESS_SLOT);
				otherShader->SetInt("u_NormalMap", NORMAL_SLOT);
				otherShader->SetMat4("u_Model", transform);
				otherShader->SetFloat4("m_color", color);
			}

			RenderCommand::DrawArrays(*sub_mesh.VertexArray, sub_mesh.numVertices);
		}
	}

	//void Renderer3D::DrawFoliage(LoadMesh& mesh, glm::mat4& transform, const glm::vec4& color, const float& material_Roughness, const float& material_metallic)
	//{
	//	glDisable(GL_CULL_FACE);
	//	//this is not ideal!! I just cannot handle shaders like this in a long run
	//	m_data->foliage_shader->Bind();//bind the textureShader
	//
	//	m_data->foliage_shader->SetFloat("Roughness", material_Roughness); //send the roughness value
	//	m_data->foliage_shader->SetFloat("Metallic", material_metallic); //send the metallic value
	//
	//	mesh.Diffuse_Texture->Bind(ALBEDO_SLOT);
	//	mesh.Roughness_Texture->Bind(ROUGHNESS_SLOT);
	//	mesh.Normal_Texture->Bind(NORMAL_SLOT);
	//
	//	m_data->foliage_shader->SetInt("u_Albedo", ALBEDO_SLOT);//bind albedo texture array to slot1;
	//	m_data->foliage_shader->SetInt("u_Roughness", ROUGHNESS_SLOT);
	//	m_data->foliage_shader->SetInt("u_NormalMap", NORMAL_SLOT);
	//	m_data->foliage_shader->SetMat4("u_Model", transform);
	//	m_data->foliage_shader->SetFloat4("m_color", color);
	//
	//	RenderCommand::DrawArrays(*mesh.VertexArray, mesh.Vertices.size());
	//	glEnable(GL_CULL_FACE);
	//	glCullFace(GL_BACK);
	//}

	void Renderer3D::DrawFoliageInstanced(LoadMesh& mesh, glm::mat4& transform, size_t instance_count, const glm::vec4& color, float TimeElapsed, const float& material_Roughness, const float& material_metallic)
	{
		for (auto& sub_mesh : mesh.m_subMeshes)
		{
			glDisable(GL_CULL_FACE);
			//m_data->foliageShader_instanced->Bind();

			m_data->foliageShader_instanced->SetFloat("Roughness", sub_mesh.m_Material->GetRoughness()); //send the roughness value
			m_data->foliageShader_instanced->SetFloat("Metallic", sub_mesh.m_Material->GetMetalness()); //send the metallic value
			
			sub_mesh.m_Material->Diffuse_Texture->Bind(ALBEDO_SLOT);
			sub_mesh.m_Material->Roughness_Texture->Bind(ROUGHNESS_SLOT);
			sub_mesh.m_Material->Normal_Texture->Bind(NORMAL_SLOT);

			m_data->foliageShader_instanced->SetInt("u_Albedo", ALBEDO_SLOT);//bind albedo texture array to slot1;
			m_data->foliageShader_instanced->SetInt("u_Roughness", ROUGHNESS_SLOT);
			m_data->foliageShader_instanced->SetInt("u_NormalMap", NORMAL_SLOT);
			m_data->foliageShader_instanced->SetMat4("u_Model", transform);
			m_data->foliageShader_instanced->SetFloat4("m_color", color);
			m_data->foliageShader_instanced->SetFloat("u_Time", TimeElapsed);
			m_data->foliageShader_instanced->SetInt("Noise", PERLIN_NOISE_TEXTURE_SLOT);
			RenderCommand::DrawInstancedArrays(*sub_mesh.VertexArray, sub_mesh.numVertices, instance_count);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		}
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

	//void Renderer3D::DrawMesh(LoadMesh& mesh, const glm::vec3& Position, const glm::vec3& Scale, const glm::vec3& rotation, const glm::vec4& color)
	//{
	//	m_data->shader->SetFloat("Roughness", 0.6f);
	//	m_data->shader->SetFloat("Metallic", 0.0f);
	//
	//	auto Rotation = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), { 1,0,0 }) *
	//		glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), { 0,1,0 }) *
	//		glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), { 0,0,1 });
	//	auto transform = glm::translate(glm::mat4(1.0f), Position) * Rotation * glm::scale(glm::mat4(1.0), Scale);
	//
	//	// waiting for the buffer
	//	GLenum waitReturn = GL_UNSIGNALED;
	//	while (waitReturn != GL_ALREADY_SIGNALED && waitReturn != GL_CONDITION_SATISFIED)
	//	{
	//		waitReturn = glClientWaitSync(syncObj, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
	//	}
	//	
	//	std::vector<int> n_meshes;
	//	for (int i = 0; i < mesh.Vertices.size(); i++)
	//		n_meshes.push_back(i);
	//
	//	std::for_each(std::execution::par, n_meshes.begin(), n_meshes.end(), [&](int i)
	//		{
	//			glm::vec3 transformed_normals = glm::normalize(glm::mat3(transform) * mesh.Normal[i]);//re-orienting the normals (do not include translation as normals only needs to be orinted)
	//			glm::vec3 transformed_tangents = glm::normalize(glm::mat3(transform) * mesh.Tangent[i]);
	//			glm::vec3 transformed_binormals = glm::normalize(glm::mat3(transform) * mesh.BiTangent[i]);
	//			m_data->Vertex[i] = (VertexAttributes(transform * glm::vec4(mesh.Vertices[i], 1), mesh.TexCoord[i], transformed_normals, transformed_tangents, transformed_binormals, mesh.Material_Index[i]));
	//			//Renderer2D::DrawLine(Quad[i].Position, (glm::vec3)Quad[i].Position + mesh.Normal[mesh.Normal_Indices[i]]*glm::vec3(2), { 0.0f,0.0f,1.0f,1.0f },2);
	//		});
	//
	//	mesh.Diffuse_Texture->Bind(ALBEDO_SLOT);
	//	mesh.Roughness_Texture->Bind(ROUGHNESS_SLOT);
	//	mesh.Normal_Texture->Bind(NORMAL_SLOT);
	//
	//	m_data->shader->SetInt("u_Albedo", ALBEDO_SLOT);//bind albedo texture array to slot1;
	//	m_data->shader->SetInt("u_Roughness", ROUGHNESS_SLOT);
	//	m_data->shader->SetInt("u_NormalMap", NORMAL_SLOT);
	//
	//	RenderCommand::DrawArrays(*m_data->va, mesh.Vertices.size());
	//
	//	// lock the buffer:
	//	glDeleteSync(syncObj);
	//	syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	//}

	ref<Shader>& Renderer3D::GetFoliageInstancedShader()
	{
		return m_data->foliageShader_instanced;
	}

	void Renderer3D::RenderScene_Deferred(Scene* scene)
	{
		SkyRenderer::RenderSky(*scene->GetCamera());
		DefferedRenderer::DeferredRenderPass();
	}

	void Renderer3D::ForwardRenderPass(Scene* scene)
	{
		DefferedRenderer::GenerateGBuffers(scene);
		RenderShadows(*scene, *scene->GetCamera());
		AmbiantOcclusion(*scene, *scene->GetCamera());
	}

}
