#include "hzpch.h"
#include "Renderer3D.h"
#include "RenderCommand.h"
#include "Buffer.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "stb_image.h"
#include "glad/glad.h"
#include "Hazel/Scene/PointLight.h"

namespace Hazel {
	EditorCamera m_camera;
	GLsync syncObj;
	glm::vec3 Renderer3D::m_SunLightDir = { 0,-2,0 };//initial light position
	unsigned int Renderer3D::depth_id = 0;
	struct VertexAttributes {
		//glm::vec3 Position;
		glm::vec4 Position;
		glm::vec2 TextureCoordinate;
		glm::vec4 Color;
		glm::vec3 Normal;
		unsigned int Material_index = 0;//serves as an index to the array of texture slot which is passed as an uniform in init()
		VertexAttributes(const glm::vec4& Position, const glm::vec2& TextureCoordinate,const glm::vec4& Color = { 1,1,1,1 },const glm::vec3& normal = {0,0,0}, unsigned int Material_index = 0)
		{
			this->Position = Position;
			this->TextureCoordinate = TextureCoordinate;
			this->Color = Color;
			this->Material_index = Material_index;
			Normal = normal;
		}
		//may more ..uv coord , tangents , normals..
	};

	struct Renderer3DStorage {
		int max_Vertices = 2000000;
		VertexAttributes* Vertex=nullptr;
		uint32_t vertexb_id;
		ref<Shadows> shadow_map;
		ref<CubeMapReflection> reflection;
		ref<Shader> shader, foliage_shader;
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

		m_data->shader = (Shader::Create("Assets/Shaders/3D_2_In_1Shader.glsl"));//texture shader
		m_data->shader->SetInt("ShadowMap", 7);//explicitly setting it
		m_data->foliage_shader = Shader::Create("Assets/Shaders/FoliageShader.glsl");//foliage shader
		m_data->foliage_shader->SetInt("ShadowMap", 7);//explicitly setting it

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//Loading cube map so that it can act as an environment light
		m_data->reflection = CubeMapReflection::Create();
		m_data->shadow_map = Shadows::Create(4096, 4096);//create a 2048x2048 shadow map
		depth_id = m_data->shadow_map->GetDepth_ID();
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		m_data->va = VertexArray::Create();

		//select IMMUTABLE buffer storage for mapping into a static buffer
		m_data->vb = VertexBuffer::Create(sizeof(VertexAttributes)*m_data->max_Vertices,BufferStorage_Type::IMMUTABLE);
		m_data->Vertex = (VertexAttributes*)m_data->vb->MapBuffer(sizeof(VertexAttributes) * m_data->max_Vertices);//do this only when BufferStorage_Type is IMMUTABLE (alternate for BufferData)

		//m_data->ib=(IndexBuffer::Create(&iba[0], sizeof(unsigned int) * iba.size()));//create the index buffer here

		m_data->bl = std::make_shared<BufferLayout>(); //buffer layout

		m_data->bl->push("position", DataType::Float4);
		m_data->bl->push("TexCoord", DataType::Float2);
		m_data->bl->push("Color", DataType::Float4);
		m_data->bl->push("Normal", DataType::Float3);
		m_data->bl->push("Material_Index", DataType::Int);
	
		m_data->va->AddBuffer(m_data->bl, m_data->vb);

		m_data->WhiteTex = Texture2D::Create(1, 1, 0xffffffff);//create a default white texture
		m_data->tex = Texture2D::Create("Assets/Textures/Test.png");
		m_data->tex->Bind(1);
		m_data->WhiteTex->Bind(0);
		//m_data->shader = (Shader::Create("Assets/Shaders/3D_2_In_1Shader.glsl"));//texture shader

		unsigned int TextureIDindex[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };

		//m_data->shader->SetIntArray("u_texture", sizeof(TextureIDindex), TextureIDindex);//pass the the array of texture slots
																						//which will be used to render textures in batch renderer
		SetSunLightDirection({ 3,-2,2});
		//unsigned int arr[] = { 11,12,13,14 };//these slots are explicitly used for all 4 seperate shadow maps
		//m_data->shader->SetIntArray("ShadowMap", 4, arr);
		syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}

	void Renderer3D::BeginScene(OrthographicCamera& camera)
	{
		m_data->shader->Bind();//bind the textureShader
		m_data->shader->SetMat4("u_ProjectionView", camera.GetProjectionViewMatix());
		m_data->shader->SetFloat3("EyePosition", camera.GetPosition());
	}

	void Renderer3D::BeginScene(Camera& camera)
	{
		m_data->shader->Bind();//bind the textureShader
		m_data->shader->SetMat4("u_ProjectionView", camera.GetProjection());
		//dont calculate specular lighting on scene camera
	}

	void Renderer3D::BeginScene(EditorCamera& camera)
	{
		//Init();
		m_data->shader->Bind();//bind the textureShader
		m_data->shader->SetMat4("u_ProjectionView", camera.GetProjectionView());//here the projection is ProjectionView
		m_data->shader->SetFloat3("EyePosition", camera.GetCameraPosition());//get the eye position for specular lighting calculation
		m_camera = camera;
		m_data->m_VertexCounter = 0;
		//Renderer2D::LineBeginScene(camera);
	}

	void Renderer3D::BeginSceneFoliage(EditorCamera& camera)
	{
		m_data->foliage_shader->Bind();//bind the textureShader
		m_data->foliage_shader->SetMat4("u_ProjectionView", camera.GetProjectionView());//here the projection is ProjectionView
		m_data->foliage_shader->SetFloat3("EyePosition", camera.GetCameraPosition());//get the eye position for specular lighting calculation
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
	}

	void Renderer3D::DrawMesh(LoadMesh& mesh,glm::mat4& transform, const glm::vec4& color, const float& material_Roughness , const float& material_metallic)
	{
		m_data->shader->SetFloat("Roughness",material_Roughness); //send the roughness value
		m_data->shader->SetFloat("Metallic", material_metallic); //send the metallic value

		// waiting for the buffer
		GLenum waitReturn = GL_UNSIGNALED;
		while (waitReturn != GL_ALREADY_SIGNALED && waitReturn != GL_CONDITION_SATISFIED)
		{
			waitReturn = glClientWaitSync(syncObj, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
		}
		m_data->m_VertexCounter = mesh.vertices.size();
		
		std::vector<int> n_meshes;
		for (int i = 0; i < m_data->m_VertexCounter; i++)
			n_meshes.push_back(i);

		std::for_each(std::execution::par, n_meshes.begin(), n_meshes.end(), [&](int i)
			{
				glm::vec3 transformed_normals = glm::normalize(glm::mat3(transform) * mesh.Normal[i]);//re-orienting the normals (do not include translation as normals only needs to be orinted)
				m_data->Vertex[i] = (VertexAttributes(transform * glm::vec4(mesh.vertices[i], 1), mesh.TexCoord[i], color, transformed_normals, mesh.Material_Index[i]));
				//Renderer2D::DrawLine(Quad[i].Position, (glm::vec3)Quad[i].Position + mesh.Normal[mesh.Normal_Indices[i]]*glm::vec3(2), { 0.0f,0.0f,1.0f,1.0f },2);
			});

		mesh.Diffuse_Texture->Bind(1);
		mesh.Roughness_Texture->Bind(3);

		m_data->shader->SetInt("u_Albedo", 1);//bind albedo texture array to slot1;
		m_data->shader->SetInt("u_Roughness", 3);

		RenderCommand::DrawArrays(*m_data->va, mesh.vertices.size());

		// lock the buffer:
		glDeleteSync(syncObj);
		syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}

	void Renderer3D::DrawFoliage(LoadMesh& mesh, glm::mat4& transform, const glm::vec4& color, const float& material_Roughness, const float& material_metallic)
	{

		m_data->foliage_shader->SetFloat("Roughness", material_Roughness); //send the roughness value
		m_data->foliage_shader->SetFloat("Metallic", material_metallic); //send the metallic value

		// waiting for the buffer
		GLenum waitReturn = GL_UNSIGNALED;
		while (waitReturn != GL_ALREADY_SIGNALED && waitReturn != GL_CONDITION_SATISFIED)
		{
			waitReturn = glClientWaitSync(syncObj, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
		}
		m_data->m_VertexCounter = mesh.vertices.size();

		std::vector<int> n_meshes;
		for (int i = 0; i < m_data->m_VertexCounter; i++)
			n_meshes.push_back(i);

		std::for_each(std::execution::par, n_meshes.begin(), n_meshes.end(), [&](int i)
			{
				glm::vec3 transformed_normals = glm::normalize(glm::mat3(transform) * mesh.Normal[i]);//re-orienting the normals (do not include translation as normals only needs to be orinted)
				m_data->Vertex[i] = (VertexAttributes(transform * glm::vec4(mesh.vertices[i], 1), mesh.TexCoord[i], color, transformed_normals, mesh.Material_Index[i]));
				//Renderer2D::DrawLine(Quad[i].Position, (glm::vec3)Quad[i].Position + mesh.Normal[mesh.Normal_Indices[i]]*glm::vec3(2), { 0.0f,0.0f,1.0f,1.0f },2);
			});

		mesh.Diffuse_Texture->Bind(1);
		mesh.Roughness_Texture->Bind(3);//explicitely bind to slot 3

		m_data->foliage_shader->SetInt("u_Albedo", 1);//bind albedo texture array to slot1;
		m_data->foliage_shader->SetInt("u_Roughness", 3);

		RenderCommand::DrawArrays(*m_data->va, mesh.vertices.size());

		// lock the buffer:
		glDeleteSync(syncObj);
		syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	}
	
	void Renderer3D::SetUpCubeMapReflections(Scene& scene)
	{
		//m_data->reflection->RenderToCubeMap(scene);
		m_data->shader->Bind();//you need to bind this other wise nothing will be rendererd
		m_data->shader->SetInt("diffuse_env", 10);//for now assign to 10 :)
		m_data->shader->SetInt("specular_env", 18);//for now assign to 10 :)

		m_data->foliage_shader->Bind();//you need to bind this other wise nothing will be rendererd
		m_data->foliage_shader->SetInt("diffuse_env", 10);//for now assign to 10 :)
		m_data->foliage_shader->SetInt("specular_env", 18);//for now assign to 10 :)

	}

	void Renderer3D::RenderShadows(Scene& scene, EditorCamera& camera)
	{
		m_data->shadow_map->RenderShadows(scene, m_SunLightDir, camera);//Light position is the light direction used for directional light
		m_data->shadow_map->PassShadowUniforms(camera, m_data->shader);
		m_data->shadow_map->PassShadowUniforms(camera, m_data->foliage_shader);
	}

	void Renderer3D::DrawMesh(LoadMesh& mesh, const glm::vec3& Position, const glm::vec3& Scale, const glm::vec3& rotation, const glm::vec4& color)
	{
		m_data->shader->SetFloat("Roughness", 0.6f);
		m_data->shader->SetFloat("Metallic", 0.0f);

		auto Rotation = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), { 1,0,0 }) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), { 0,1,0 }) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), { 0,0,1 });
		auto transform = glm::translate(glm::mat4(1.0f), Position) * Rotation * glm::scale(glm::mat4(1.0), Scale);

		// waiting for the buffer
		GLenum waitReturn = GL_UNSIGNALED;
		while (waitReturn != GL_ALREADY_SIGNALED && waitReturn != GL_CONDITION_SATISFIED)
		{
			waitReturn = glClientWaitSync(syncObj, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
		}
		
		std::vector<int> n_meshes;
		for (int i = 0; i < mesh.vertices.size(); i++)
			n_meshes.push_back(i);

		std::for_each(std::execution::par, n_meshes.begin(), n_meshes.end(), [&](int i)
			{

				glm::vec3 transformed_normals = glm::normalize(glm::mat3(transform) * mesh.Normal[i]);//re-orienting the normals (do not include translation as normals only needs to be orinted)
				m_data->Vertex[i] = (VertexAttributes(transform * glm::vec4(mesh.vertices[i], 1), mesh.TexCoord[i], color, transformed_normals, mesh.Material_Index[i]));
				//Renderer2D::DrawLine(Quad[i].Position, (glm::vec3)Quad[i].Position + mesh.Normal[mesh.Normal_Indices[i]]*glm::vec3(2), { 0.0f,0.0f,1.0f,1.0f },2);
			});

		mesh.Diffuse_Texture->Bind(1);
		mesh.Roughness_Texture->Bind(3);

		m_data->shader->SetInt("u_Albedo", 1);//bind albedo texture array to slot1;
		m_data->shader->SetInt("u_Roughness", 3);


		RenderCommand::DrawArrays(*m_data->va, mesh.vertices.size());

		// lock the buffer:
		glDeleteSync(syncObj);
		syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}

}
