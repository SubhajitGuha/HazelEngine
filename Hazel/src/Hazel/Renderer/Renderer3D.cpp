#include "hzpch.h"
#include "Renderer3D.h"
#include "RenderCommand.h"
#include "Buffer.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glad/glad.h"

namespace Hazel {
	EditorCamera m_camera;
	struct VertexAttributes {
		//glm::vec3 Position;
		glm::vec4 Position;
		glm::vec2 TextureCoordinate;
		glm::vec4 Color;
		glm::vec3 Normal;
		unsigned int TextureSlotindex = 0;//serves as an index to the array of texture slot which is passed as an uniform in init()
		VertexAttributes(const glm::vec4& Position, const glm::vec2& TextureCoordinate,const glm::vec4& Color = { 1,1,1,1 }, unsigned int TextureSlotindex = 0,const glm::vec3& normal = {0,0,0})
		{
			this->Position = Position;
			this->TextureCoordinate = TextureCoordinate;
			this->Color = Color;
			this->TextureSlotindex = TextureSlotindex;
			Normal = normal;
		}
		//may more ..uv coord , tangents , normals..
	};

	struct Renderer3DStorage {
		int max_Vertices = 10000;

		ref<Shader> shader;
		ref<Texture2D> WhiteTex,tex;
		ref<BufferLayout> bl;

		uint32_t m_VertexCounter = 0;
	};
	static Renderer3DStorage* m_data;

	void Renderer3D::Init()
	{
		m_data = new Renderer3DStorage;
		
		m_data->bl = std::make_shared<BufferLayout>(); //buffer layout

		m_data->bl->push("position", DataType::Float4);
		m_data->bl->push("TexCoord", DataType::Float2);
		m_data->bl->push("Color", DataType::Float4);
		m_data->bl->push("Normal", DataType::Float3);
		m_data->bl->push("TextureSlot", DataType::Int);

		m_data->WhiteTex = Texture2D::Create(1, 1, 0xffffffff);//create a default white texture
		m_data->tex = Texture2D::Create("Assets/Textures/Test.png");
		m_data->shader = (Shader::Create("Assets/Shaders/3D_2_In_1Shader.glsl"));//texture shader

		unsigned int TextureIDindex[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };

		m_data->shader->SetIntArray("u_Texture", sizeof(TextureIDindex), TextureIDindex);//pass the the array of texture slots
																						//which will be used to render textures in batch renderer
		SetLightPosition({ 3,-2,2});
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
		//Renderer2D::LineBeginScene(camera);
	}

	void Renderer3D::EndScene()
	{
	}

	void Renderer3D::SetLightPosition(const glm::vec3& pos)
	{
		m_data->shader->SetFloat3("LightPosition", pos);
	}

	void Renderer3D::DrawMesh(LoadMesh& mesh)
	{
		std::vector< VertexAttributes> Quad(mesh.Vertex_Indices.size(), { glm::vec4(0.0),glm::vec2(0.0)});
		std::vector<unsigned int> iba;
		auto transform = glm::scale(glm::mat4(1.0), { 2,2,2 });

		for (int i = 0; i < mesh.Vertex_Indices.size(); i++)
		{
			glm::vec3 transformed_normals = glm::normalize(glm::mat3(transform) * mesh.Normal[mesh.Normal_Indices[i]]);
			Quad[i] = (VertexAttributes(transform * glm::vec4(mesh.vertices[mesh.Vertex_Indices[i]], 1), mesh.TexCoord[mesh.TexCoord_Indices[i]], {1,1,1,1}, 0, transformed_normals));
			//Renderer2D::DrawLine(Quad[i].Position, (glm::vec3)Quad[i].Position + mesh.Normal[mesh.Normal_Indices[i]]*glm::vec3(2), { 0.0f,0.0f,1.0f,1.0f },2);
		}

		for (int i = 0; i < mesh.Vertex_Indices.size(); i++)
			iba.push_back(i);

		ref<VertexArray> vao = VertexArray::Create();

		ref<VertexBuffer> vb(VertexBuffer::Create(&Quad[0].Position.x, sizeof(VertexAttributes) * Quad.size()));
		//ref<IndexBuffer> ib(IndexBuffer::Create(&mesh.Vertex_Indices[0], sizeof(unsigned int) * mesh.Vertex_Indices.size()));//create the index buffer here
		ref<IndexBuffer> ib(IndexBuffer::Create(&iba[0], sizeof(unsigned int) * iba.size()));//create the index buffer here


		vao->AddBuffer(m_data->bl, vb);
		vao->SetIndexBuffer(ib);

		//m_data->tex->Bind(1);
		m_data->WhiteTex->Bind(0);

		//vb->SetData(sizeof(VertexAttributes) * m_data->Quad.size(), &m_data->Quad[0].Position.x);
		//Renderer2D::LineEndScene();
		RenderCommand::DrawIndex(*vao);
	}

	void Renderer3D::DrawMesh(LoadMesh& mesh,glm::mat4& transform, const glm::vec4& color)
	{
		//m_data->shader->SetMat4("u_ModelTransform", transform);
		std::vector< VertexAttributes> Quad(mesh.Vertex_Indices.size(), { glm::vec4(0.0),glm::vec2(0.0) });
		std::vector<unsigned int> iba;

		for (int i = 0; i < mesh.Vertex_Indices.size(); i++)
		{
			glm::vec3 transformed_normals = glm::normalize(glm::mat3(transform) * mesh.Normal[mesh.Normal_Indices[i]]);//re-orienting the normals (do not include translation as normals only needs to be orinted)
			Quad[i] = (VertexAttributes(transform * glm::vec4(mesh.vertices[mesh.Vertex_Indices[i]], 1), mesh.TexCoord[mesh.TexCoord_Indices[i]], color, 0, transformed_normals));
			//Renderer2D::DrawLine(Quad[i].Position, (glm::vec3)Quad[i].Position + mesh.Normal[mesh.Normal_Indices[i]]*glm::vec3(2), { 0.0f,0.0f,1.0f,1.0f },2);
		}

		for (int i = 0; i < mesh.Vertex_Indices.size(); i++)
			iba.push_back(i);

		ref<VertexArray> vao = VertexArray::Create();

		ref<VertexBuffer> vb(VertexBuffer::Create(&Quad[0].Position.x, sizeof(VertexAttributes) * Quad.size()));
		//ref<IndexBuffer> ib(IndexBuffer::Create(&mesh.Vertex_Indices[0], sizeof(unsigned int) * mesh.Vertex_Indices.size()));//create the index buffer here
		ref<IndexBuffer> ib(IndexBuffer::Create(&iba[0], sizeof(unsigned int) * iba.size()));//create the index buffer here


		vao->AddBuffer(m_data->bl, vb);
		vao->SetIndexBuffer(ib);

		//m_data->tex->Bind(1);
		m_data->WhiteTex->Bind(0);

		//vb->SetData(sizeof(VertexAttributes) * m_data->Quad.size(), &m_data->Quad[0].Position.x);
		//Renderer2D::LineEndScene();
		RenderCommand::DrawIndex(*vao);
	}

	void Renderer3D::DrawMesh(LoadMesh& mesh, const glm::vec3& Position, const glm::vec3& Scale, const glm::vec3& rotation, const glm::vec4& color)
	{
		std::vector< VertexAttributes> Quad(mesh.Vertex_Indices.size(), { glm::vec4(0.0),glm::vec2(0.0) });
		std::vector<unsigned int> iba;

		auto Rotation = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), { 1,0,0 }) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), { 0,1,0 }) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), { 0,0,1 });
		auto transform = glm::translate(glm::mat4(1.0f), Position) * Rotation * glm::scale(glm::mat4(1.0), Scale);

		for (int i = 0; i < mesh.Vertex_Indices.size(); i++)
		{
			glm::vec3 transformed_normals = glm::normalize(glm::mat3(transform) * mesh.Normal[mesh.Normal_Indices[i]]);
			Quad[i] = (VertexAttributes(transform * glm::vec4(mesh.vertices[mesh.Vertex_Indices[i]], 1), mesh.TexCoord[mesh.TexCoord_Indices[i]], color, 0, transformed_normals));
			//Renderer2D::DrawLine(Quad[i].Position, (glm::vec3)Quad[i].Position + mesh.Normal[mesh.Normal_Indices[i]]*glm::vec3(2), { 0.0f,0.0f,1.0f,1.0f },2);
		}

		for (int i = 0; i < mesh.Vertex_Indices.size(); i++)
			iba.push_back(i);

		ref<VertexArray> vao = VertexArray::Create();

		ref<VertexBuffer> vb(VertexBuffer::Create(&Quad[0].Position.x, sizeof(VertexAttributes) * Quad.size()));
		//ref<IndexBuffer> ib(IndexBuffer::Create(&mesh.Vertex_Indices[0], sizeof(unsigned int) * mesh.Vertex_Indices.size()));//create the index buffer here
		ref<IndexBuffer> ib(IndexBuffer::Create(&iba[0], sizeof(unsigned int) * iba.size()));//create the index buffer here


		vao->AddBuffer(m_data->bl, vb);
		vao->SetIndexBuffer(ib);

		//m_data->tex->Bind(1);
		m_data->WhiteTex->Bind(0);

		//vb->SetData(sizeof(VertexAttributes) * m_data->Quad.size(), &m_data->Quad[0].Position.x);
		//Renderer2D::LineEndScene();
		RenderCommand::DrawIndex(*vao);
	}

}
