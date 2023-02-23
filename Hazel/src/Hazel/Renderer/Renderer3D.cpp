#include "hzpch.h"
#include "Renderer3D.h"
#include "RenderCommand.h"
#include "Buffer.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glad/glad.h"

namespace Hazel {
	struct VertexAttributes {
		//glm::vec3 Position;
		glm::vec4 Position;
		glm::vec2 TextureCoordinate;
		glm::vec4 Color;
		unsigned int TextureSlotindex = 0;//serves as an index to the array of texture slot which is passed as an uniform in init()
		VertexAttributes(glm::vec4 Position, glm::vec2 TextureCoordinate, glm::vec4 Color = { 1,1,1,1 }, unsigned int TextureSlotindex = 0)
		{
			this->Position = Position;
			this->TextureCoordinate = TextureCoordinate;
			this->Color = Color;
			this->TextureSlotindex = TextureSlotindex;
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
		m_data->bl->push("TextureSlot", DataType::Int);

		m_data->WhiteTex = Texture2D::Create(1, 1, 0xffffffff);//create a default white texture
		m_data->tex = Texture2D::Create("Assets/Textures/Test.png");
		m_data->shader = (Shader::Create("Assets/Shaders/3D_2_In_1Shader.glsl"));//texture shader

		unsigned int TextureIDindex[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };

		m_data->shader->SetIntArray("u_Texture", sizeof(TextureIDindex), TextureIDindex);//pass the the array of texture slots
																						//which will be used to render textures in batch renderer
	}

	void Renderer3D::BeginScene(OrthographicCamera& camera)
	{
		m_data->shader->Bind();//bind the textureShader
		m_data->shader->SetMat4("u_ProjectionView", camera.GetProjectionViewMatix());

	}

	void Renderer3D::BeginScene(Camera& camera)
	{
		m_data->shader->Bind();//bind the textureShader
		m_data->shader->SetMat4("u_ProjectionView", camera.GetProjection());

	}

	void Renderer3D::BeginScene(EditorCamera& camera)
	{
		//Init();
		m_data->shader->Bind();//bind the textureShader
		m_data->shader->SetMat4("u_ProjectionView", camera.GetProjectionView());//here the projection is ProjectionView

	}

	void Renderer3D::EndScene()
	{
	}

	void Renderer3D::DrawMesh(LoadMesh& mesh)
	{
		std::vector< VertexAttributes> Quad(mesh.vertices.size(), { glm::vec4(0.0),glm::vec2(0.0),glm::vec4(0.0)});
		auto transform = glm::scale(glm::mat4(1.0), { 2,2,2 });

		for (int i = 0; i < mesh.vertices.size(); i++)
		{
			Quad[i] = (VertexAttributes(transform * glm::vec4(mesh.vertices[i],1), mesh.TexCoord[i], { 1,1,1,1 },1));
		}
		for (int i = 0; i < mesh.Vertex_Indices.size(); i++)
		{
			Quad[mesh.Vertex_Indices[i]].TextureCoordinate = mesh.TexCoord[mesh.TexCoord_Indices[i]];
		}
		ref<VertexArray> vao= VertexArray::Create();

		ref<VertexBuffer> vb (VertexBuffer::Create(&Quad[0].Position.x , sizeof(VertexAttributes) * Quad.size()));
		ref<IndexBuffer> ib(IndexBuffer::Create(&mesh.Vertex_Indices[0],sizeof(unsigned int) * mesh.Vertex_Indices.size()));//create the index buffer here
		

		vao->AddBuffer(m_data->bl, vb);
		vao->SetIndexBuffer(ib);

		m_data->tex->Bind(1);
		//m_data->WhiteTex->Bind(0);

		//vb->SetData(sizeof(VertexAttributes) * m_data->Quad.size(), &m_data->Quad[0].Position.x);
		RenderCommand::DrawIndex(*vao);
	}

}
