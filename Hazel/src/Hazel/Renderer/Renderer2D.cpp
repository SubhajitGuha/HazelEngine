#include "hzpch.h"
#include "Renderer2D.h"
#include "RenderCommand.h"
#include "Buffer.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glad/glad.h"

#define M_TRANSFORM(pos,angle) glm::rotate(glm::mat4(1),glm::radians(angle),{0,0,1})\
 * glm::scale(glm::mat4(1),{1,1,1}) * pos
namespace Hazel {

	struct VertexAttributes {
		//glm::vec3 Position;
		glm::vec4 Position;
		glm::vec2 TextureCoordinate;
		glm::vec4 Color;
		unsigned int TextureSlotindex = 0;//serves as an index to the array of texture slot which is passed as an uniform in init()
		VertexAttributes(glm::vec4 Position, glm::vec2 TextureCoordinate, glm::vec4 Color = {1,1,1,1}, unsigned int TextureSlotindex = 0)
		{
			this->Position = Position;
			this->TextureCoordinate = TextureCoordinate;
			this->Color = Color;
			this->TextureSlotindex = TextureSlotindex;
		}
		//may more ..uv coord , tangents , normals..
	};

	struct Renderer2DStorage {
		int maxQuads = 100000;
		int NumIndices = maxQuads * 6;
		int NumVertices = maxQuads * 4;
		
		ref<VertexArray> vao;
		ref<Shader> shader;
		ref<Texture2D> WhiteTex;
		ref<VertexBuffer> vb;
		//ref<Texture2D> texture, texture2;

		std::vector< VertexAttributes> Quad;
		std::vector<unsigned int> index;
		int m_VertexCounter = 0;
	};

		static Renderer2DStorage* m_data;

	void Renderer2D::Init()
	{
		m_data = new Renderer2DStorage;
		
		//initilize the vertex buffer data and index buffer data
		m_data->Quad.resize(m_data->NumVertices, { glm::vec4(0.0),glm::vec2(0.0),glm::vec4(0.0) });
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

		bl->push("position", DataType::Float4);
		bl->push("TexCoord", DataType::Float2);
		bl->push("Color", DataType::Float4);
		bl->push("TextureSlot", DataType::Int);

		ref<IndexBuffer> ib(IndexBuffer::Create(&m_data->index[0], m_data->NumIndices));
		m_data->vao->AddBuffer(bl, m_data->vb);
		m_data->vao->SetIndexBuffer(ib);

		m_data->WhiteTex = Texture2D::Create(1,1,0xffffffff);//create a default white texture
		m_data->shader = (Shader::Create("Assets/Shaders/2_In_1Shader.glsl"));//texture shader
		
		unsigned int TextureIDindex[] = { 0,1,2,3,4,5,6,7 };

		m_data->shader->SetIntArray("u_Texture", sizeof(TextureIDindex), TextureIDindex);//pass the the array of texture slots
																						//which will be used to render textures in batch renderer

	}
	void Renderer2D::BeginScene(OrthographicCamera& camera)
	{
		m_data->shader->Bind();//bind the textureShader
		m_data->shader->SetMat4("u_ProjectionView", camera.GetProjectionViewMatix());
	}

	void Renderer2D::EndScene()
	{
		m_data->vb->SetData(sizeof(VertexAttributes)*4* m_data->m_VertexCounter, &m_data->Quad[0].Position);
		RenderCommand::DrawIndex(*m_data->vao);//draw call done only once (batch rendering is implemented)
		m_data->m_VertexCounter = 0;
	}

	void Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const glm::vec4& col, const float angle)
	{
		m_data->Quad[m_data->m_VertexCounter+0] = VertexAttributes(M_TRANSFORM(glm::vec4(pos,1),angle),{0.0,0.0},col );
		m_data->Quad[m_data->m_VertexCounter+1] = VertexAttributes(M_TRANSFORM(glm::vec4(pos.x ,pos.y + scale.y,pos.z,1.f), angle),{0.0,1.0},col);
		m_data->Quad[m_data->m_VertexCounter+2] = VertexAttributes(M_TRANSFORM(glm::vec4(pos.x + scale.x,pos.y+scale.y,pos.z,1.f), angle),{1.0,1.0},col );
		m_data->Quad[m_data->m_VertexCounter+3] = VertexAttributes(M_TRANSFORM(glm::vec4(pos.x+scale.x,pos.y,pos.z,1.f), angle),{1.0,0.0},col );

		m_data->WhiteTex->Bind(0);//bind the white texture so that solid color is selected in fragment shader
		//m_data->shader->SetFloat4("u_color", col);

		m_data->m_VertexCounter+=4;
	}

	void Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec3& scale, ref<Texture2D> tex , unsigned int index, const float angle)
	{
			m_data->Quad[m_data->m_VertexCounter + 0] = VertexAttributes(M_TRANSFORM(glm::vec4(pos, 1), angle), { 0,0 }, { 1,1,1,1 }, index);
			m_data->Quad[m_data->m_VertexCounter + 1] = VertexAttributes(M_TRANSFORM(glm::vec4(pos.x, pos.y + scale.y, pos.z, 1.f), angle), { 0,1 }, { 1,1,1,1 }, index);
			m_data->Quad[m_data->m_VertexCounter + 2] = VertexAttributes(M_TRANSFORM(glm::vec4(pos.x + scale.x, pos.y + scale.y, pos.z, 1.f), angle), { 1,1 }, { 1,1,1,1 }, index);
			m_data->Quad[m_data->m_VertexCounter + 3] = VertexAttributes(M_TRANSFORM(glm::vec4(pos.x + scale.x, pos.y, pos.z, 1.f), angle), { 1,0 }, { 1,1,1,1 }, index);

		
		tex->Bind(index);
		//m_data->shader->SetFloat4("u_color", glm::vec4(1));//set the multiplying color to white so that the texture is selected in fragment shader
		
		m_data->m_VertexCounter += 4;

	}
	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color)
	{
		m_data->Quad[m_data->m_VertexCounter + 0] = VertexAttributes(transform*glm::vec4(0,0,0,1), { 0,0 }, color);
		m_data->Quad[m_data->m_VertexCounter + 1] = VertexAttributes(transform*glm::vec4(0,1,0,1), { 0,1 }, color);
		m_data->Quad[m_data->m_VertexCounter + 2] = VertexAttributes(transform*glm::vec4(1,1,0,1), { 1,1 }, color);
		m_data->Quad[m_data->m_VertexCounter + 3] = VertexAttributes(transform*glm::vec4(1,0,0,1), { 1,0 }, color);

		m_data->WhiteTex->Bind(0);//There is no need to bind the texture every frame .In this case the texture can be bound once and used all the time
		m_data->m_VertexCounter += 4;
	}
	void Renderer2D::DrawSubTexturedQuad(const glm::vec3& pos, const glm::vec3& scale, ref<SubTexture2D> tex, unsigned int index, const float angle)
	{
		m_data->Quad[m_data->m_VertexCounter + 0] = VertexAttributes(M_TRANSFORM(glm::vec4(pos, 1), angle), tex->m_TextureCoordinate[0], { 1,1,1,1 }, index);
		m_data->Quad[m_data->m_VertexCounter + 1] = VertexAttributes(M_TRANSFORM(glm::vec4(pos.x, pos.y + scale.y, pos.z, 1.f), angle), tex->m_TextureCoordinate[1], { 1,1,1,1 }, index);
		m_data->Quad[m_data->m_VertexCounter + 2] = VertexAttributes(M_TRANSFORM(glm::vec4(pos.x + scale.x, pos.y + scale.y, pos.z, 1.f), angle), tex->m_TextureCoordinate[2], { 1,1,1,1 }, index);
		m_data->Quad[m_data->m_VertexCounter + 3] = VertexAttributes(M_TRANSFORM(glm::vec4(pos.x + scale.x, pos.y, pos.z, 1.f), angle), tex->m_TextureCoordinate[3], { 1,1,1,1 }, index);

		tex->Texture->Bind(index);//There is no need to bind the texture every frame .In this case the texture can be bound once and used all the time
		m_data->m_VertexCounter += 4;
	}
}