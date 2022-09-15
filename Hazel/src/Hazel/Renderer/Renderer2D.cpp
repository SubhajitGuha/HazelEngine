#include "hzpch.h"
#include "Renderer2D.h"
#include "RenderCommand.h"
#include "Buffer.h"
#include "glm/gtc/matrix_transform.hpp"
namespace Hazel {
	struct Renderer2DStorage {
		ref<VertexArray> vao;
		ref<Shader> shader;
		ref<Texture2D> WhiteTex;
	};

	static Renderer2DStorage* m_data = new Renderer2DStorage;
	
	void Renderer2D::Init()
	{
		float pos[] =
		{ 0.5,0.5,0.0,   1.0,1.0,
		0.5,-0.5,0.0,   1.0,0.0,
		-0.5,-0.5,0.0,  0.0,0.0,
		-0.5,0.5,0.0,   0.0,1.0 };

		unsigned int index[] =
		{ 0,1,2,
		0,2,3 };


		m_data->vao = (VertexArray::Create());//vertex array
		ref<VertexBuffer> vb(VertexBuffer::Create(pos, sizeof(pos)));//vertex buffer
		ref<BufferLayout> bl = std::make_shared<BufferLayout>(); //buffer layout
		bl->push("position", DataType::Float3);
		bl->push("TexCoord", DataType::Float2);
		ref<IndexBuffer> ib(IndexBuffer::Create(index, sizeof(index)));
		m_data->vao->AddBuffer(bl, vb);
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
	}
	void Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const glm::vec4& col)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0), pos) * glm::scale(glm::mat4(1), scale);
		m_data->shader->SetMat4("u_ModelTransform", transform);

		m_data->WhiteTex->Bind(0);//bind the white texture so that solid color is selected in fragment shader
		m_data->shader->SetFloat4("u_color", col);
		RenderCommand::DrawIndex(*m_data->vao);
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