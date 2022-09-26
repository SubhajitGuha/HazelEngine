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
		int maxQuads = 100;

		ref<VertexArray> vao;
		ref<Shader> shader;
		ref<Texture2D> WhiteTex;
		ref<VertexBuffer> vb;

		float pos[36 * 3] =
		{ 0,0,0,0,0,1,1,1,1,
		1,0,0,1,0,1,1,1,1,
		1,1,0,1,1,1,1,1,1,
		0,1,0,0,1,1,1,1,1,
		0.5,0.5,0,0,0,1,1,1,1,
		1.5,0.5,0,1,0,1,1,1,1,
		1.5,1.5,0,1,1,1,1,1,1,
		0.5,1.5,0,0,1,1,1,1,1,
		2,2,0,0,0,1,1,1,1,
		3,2,0,1,0,1,1,1,1,
		3,3,0,1,1,1,1,1,1,
		2, 3, 0, 0, 1, 1, 1, 1, 1, };

		std::vector< std::vector<VertexAttributes> > Quad;
		unsigned int m_QuadCunter = 0;
	};
		static Renderer2DStorage* m_data;
	void Renderer2D::Init()
	{
		m_data = new Renderer2DStorage;

		unsigned int index[] =
		{0,1,2,0,3,2,4,5,6,4,7,6,8,9,10,8,11,9
		};

		m_data->vao=(VertexArray::Create());//vertex array
		//ref<VertexBuffer> vb = VertexBuffer::Create(pos, sizeof(pos));//vertex buffer
		m_data->vb = VertexBuffer::Create((sizeof(VertexAttributes)*4)*m_data->maxQuads);//(sizeof(VertexAttributes)*4) gives one quad multiply it with howmany quads you want
		ref<BufferLayout> bl = std::make_shared<BufferLayout>(); //buffer layout
		bl->push("position", DataType::Float3);
		bl->push("TexCoord", DataType::Float2);
		bl->push("Color", DataType::Float4);
		ref<IndexBuffer> ib(IndexBuffer::Create(index, sizeof(index)));
		m_data->vao->AddBuffer(bl, m_data->vb);
		m_data->vao->SetIndexBuffer(ib);

		m_data->WhiteTex = Texture2D::Create(1,1,0xffffffff);//create a default white texture
		m_data->shader = (Shader::Create("Assets/Shaders/2_In_1Shader.glsl"));//texture shader

		m_data->Quad.resize(m_data->maxQuads, std::vector<VertexAttributes>(4, {glm::vec3(0.0),glm::vec2(0.0),glm::vec4(0.0)}));

	}
	void Renderer2D::BeginScene(OrthographicCamera& camera)
	{
		m_data->shader->Bind();//bind the textureShader
		m_data->shader->SetInt("u_Texture", 0);//bind the texture to slot 0
		m_data->shader->SetMat4("u_ProjectionView", camera.GetProjectionViewMatix());
	}
	void Renderer2D::EndScene()
	{
		m_data->vb->SetData(sizeof(VertexAttributes)*4* m_data->m_QuadCunter, m_data->pos);
		
		//auto var = sizeof(VertexAttributes) * 4 * m_data->m_QuadCunter;
		RenderCommand::DrawIndex(*m_data->vao);
		m_data->m_QuadCunter = 0;
	}
	void Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const glm::vec4& col)
	{
		m_data->Quad[m_data->m_QuadCunter][0] = { pos,{0.0,0.0},col };
		m_data->Quad[m_data->m_QuadCunter][1] = { {pos.x ,pos.y + scale.y,pos.z},{0.0,1.0},col };
		m_data->Quad[m_data->m_QuadCunter][2] = { {pos.x + scale.x,pos.y+scale.y,pos.z},{1.0,1.0},col };
		m_data->Quad[m_data->m_QuadCunter][3] = { {pos.x+scale.x,pos.y,pos.z},{1.0,0.0},col };

		//glm::mat4 transform = glm::translate(glm::mat4(1.0), pos) * glm::scale(glm::mat4(1), scale);
		glm::mat4 transform = glm::mat4(1.0);
		m_data->shader->SetMat4("u_ModelTransform", transform);

		m_data->WhiteTex->Bind(0);//bind the white texture so that solid color is selected in fragment shader
		m_data->shader->SetFloat4("u_color", col);
		//RenderCommand::DrawIndex(*m_data->vao);

		++m_data->m_QuadCunter;
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