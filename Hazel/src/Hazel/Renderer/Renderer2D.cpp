#include "hzpch.h"
#include "Renderer2D.h"
#include "RenderCommand.h"
#include "Buffer.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glad/glad.h"

namespace Hazel {

	struct VertexAttributes {
		glm::vec3 Position;
		glm::vec2 TextureCoordinate;
		glm::vec4 Color;
		//may more ..uv coord , tangents , normals..
	};

	struct Renderer2DStorage {
		int maxQuads = 10000;
		int NumIndices = maxQuads * 6;
		int NumVertices = maxQuads * 4;

		ref<VertexArray> vao;
		ref<Shader> shader;
		ref<Texture2D> WhiteTex;
		ref<VertexBuffer> vb;

		std::vector< VertexAttributes> Quad;
		std::vector<unsigned int> index;
		unsigned int m_VertexCounter = 0;
	};

		static Renderer2DStorage* m_data;

	void Renderer2D::Init()
	{
		m_data = new Renderer2DStorage;

		//initilize the vertex buffer data and index buffer data
		m_data->Quad.resize(m_data->NumVertices, { glm::vec3(0.0),glm::vec2(0.0),glm::vec4(0.0) });
		m_data->index.resize(m_data->NumIndices);

		int offset = 0;
		for (unsigned int i = 0; i < m_data->NumIndices; i+=6)
		{
			m_data->index[i] = offset;
			m_data->index[i+1] = offset+1;
			m_data->index[i+2] = offset+2;
			m_data->index[i+3] = offset;
			m_data->index[i+4] = offset+3;
			m_data->index[i+5] = offset+2;
			offset += 4;
		}

		m_data->vao=(VertexArray::Create());//vertex array
		//ref<VertexBuffer> vb = VertexBuffer::Create(pos, sizeof(pos));//vertex buffer
		m_data->vb = VertexBuffer::Create((sizeof(VertexAttributes)*4)*m_data->maxQuads);//(sizeof(VertexAttributes)*4) gives one quad multiply it with howmany quads you want
		ref<BufferLayout> bl = std::make_shared<BufferLayout>(); //buffer layout
		bl->push("position", DataType::Float3);
		bl->push("TexCoord", DataType::Float2);
		bl->push("Color", DataType::Float4);
		ref<IndexBuffer> ib(IndexBuffer::Create(&m_data->index[0], m_data->NumIndices));
		m_data->vao->AddBuffer(bl, m_data->vb);
		m_data->vao->SetIndexBuffer(ib);

		m_data->WhiteTex = Texture2D::Create(1,1,0xffffffff);//create a default white texture
		m_data->shader = (Shader::Create("Assets/Shaders/2_In_1Shader.glsl"));//texture shader

		

	}
	void Renderer2D::BeginScene(OrthographicCamera& camera)
	{
		m_data->shader->Bind();//bind the textureShader
		m_data->shader->SetInt("u_Texture", 0);//bind the texture to slot 0
		m_data->shader->SetMat4("u_ProjectionView", camera.GetProjectionViewMatix());
	}

	void Renderer2D::EndScene()
	{
		m_data->vb->SetData(sizeof(VertexAttributes)*4* m_data->m_VertexCounter, &m_data->Quad[0].Position);
		RenderCommand::DrawIndex(*m_data->vao);
		m_data->m_VertexCounter = 0;
	}

	void Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const glm::vec4& col)
	{
		m_data->Quad[m_data->m_VertexCounter+0] = { pos,{0.0,0.0},col };
		m_data->Quad[m_data->m_VertexCounter+1] = { {pos.x ,pos.y + scale.y,pos.z},{0.0,1.0},col };
		m_data->Quad[m_data->m_VertexCounter+2] = { {pos.x + scale.x,pos.y+scale.y,pos.z},{1.0,1.0},col };
		m_data->Quad[m_data->m_VertexCounter+3] = { {pos.x+scale.x,pos.y,pos.z},{1.0,0.0},col };

		//glm::mat4 transform = glm::translate(glm::mat4(1.0), pos) * glm::scale(glm::mat4(1), scale);
		glm::mat4 transform = glm::mat4(1.0);
		m_data->shader->SetMat4("u_ModelTransform", transform);

		m_data->WhiteTex->Bind(0);//bind the white texture so that solid color is selected in fragment shader
		m_data->shader->SetFloat4("u_color", col);

		m_data->m_VertexCounter+=4;
	}

	void Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec3& scale, ref<Texture2D> tex)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0), pos) * glm::scale(glm::mat4(1), scale);
		m_data->shader->SetMat4("u_ModelTransform", transform);
		
		m_data->shader->SetFloat4("u_color", glm::vec4(1));//set the multiplying color to white so that the texture is selected in fragment shader

		tex->Bind(0);
		RenderCommand::DrawIndex(*m_data->vao);
	}
}