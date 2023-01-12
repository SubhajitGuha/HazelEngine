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

	struct LineAttributes {
		//attributes for drawing a line
		glm::vec4 Position;
		glm::vec4 Color;
		LineAttributes() = default;
		LineAttributes(const glm::vec4& p1, const glm::vec4& c)
		{
			Position = p1;
			Color = c;
		}
	};

	struct Renderer2DStorage {
		int maxQuads = 1000000;
		int NumIndices = maxQuads * 6;
		int NumVertices = maxQuads * 4;
		
		ref<VertexArray> vao;
		ref<VertexArray> Linevao;
		ref<Shader> shader,Lineshader;
		ref<Texture2D> WhiteTex;
		ref<VertexBuffer> vb;
		ref<VertexBuffer> Linevb;
		//ref<Texture2D> texture, texture2;

		std::vector< VertexAttributes> Quad;
		std::vector<LineAttributes> Line;
		std::vector<unsigned int> index;
		uint32_t m_VertexCounter = 0;
		uint32_t m_LineVertCounter = 0;
	};
		static Renderer2DStorage* m_data;

	void Renderer2D::Init()
	{
		m_data = new Renderer2DStorage;
		
		//initilize the vertex buffer data and index buffer data
		m_data->Quad.resize(m_data->NumVertices*4, { glm::vec4(0.0),glm::vec2(0.0),glm::vec4(0.0) });
		m_data->Line.resize(m_data->maxQuads*2);//allocate space for the vertices that will draw the line
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

		m_data->Linevao = (VertexArray::Create());
		m_data->vao=(VertexArray::Create());//vertex array
		
		m_data->Linevb = VertexBuffer::Create((sizeof(LineAttributes)*2)* m_data->maxQuads);
		m_data->vb = VertexBuffer::Create((sizeof(VertexAttributes)*4)*m_data->maxQuads);//(sizeof(VertexAttributes)*4) gives one quad multiply it with howmany quads you want

		ref<BufferLayout> Linebl = std::make_shared<BufferLayout>(); //buffer layout
		ref<BufferLayout> bl = std::make_shared<BufferLayout>(); //buffer layout

		Linebl->push("position", DataType::Float4);
		Linebl->push("Color", DataType::Float4);

		bl->push("position", DataType::Float4);
		bl->push("TexCoord", DataType::Float2);
		bl->push("Color", DataType::Float4);
		bl->push("TextureSlot", DataType::Int);

		m_data->Linevao->AddBuffer(Linebl, m_data->Linevb);
		
		ref<IndexBuffer> ib(IndexBuffer::Create(&m_data->index[0], m_data->NumIndices));
		m_data->vao->AddBuffer(bl, m_data->vb);
		m_data->vao->SetIndexBuffer(ib);


		m_data->WhiteTex = Texture2D::Create(1,1,0xffffffff);//create a default white texture
		m_data->Lineshader = (Shader::Create("Assets/Shaders/LineShader.glsl"));
		m_data->shader = (Shader::Create("Assets/Shaders/2_In_1Shader.glsl"));//texture shader

		unsigned int TextureIDindex[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};

		m_data->shader->SetIntArray("u_Texture", sizeof(TextureIDindex), TextureIDindex);//pass the the array of texture slots
																						//which will be used to render textures in batch renderer

	}
	void Renderer2D::BeginScene(OrthographicCamera& camera)
	{
		m_data->shader->Bind();//bind the textureShader
		m_data->shader->SetMat4("u_ProjectionView", camera.GetProjectionViewMatix());
	}

	void Renderer2D::BeginScene(Camera& camera)
	{
		m_data->shader->Bind();//bind the textureShader
		m_data->shader->SetMat4("u_ProjectionView", camera.GetProjection());
	}

	void Renderer2D::EndScene()
	{
		m_data->vb->SetData(sizeof(VertexAttributes)*4* m_data->m_VertexCounter, &m_data->Quad[0].Position);
		RenderCommand::DrawIndex(*m_data->vao);//draw call done only once (batch rendering is implemented)
		m_data->m_VertexCounter = 0;
	}

	void Renderer2D::LineBeginScene(OrthographicCamera& camera)
	{
		m_data->Lineshader->Bind();
		m_data->Lineshader->SetMat4("u_ProjectionView", camera.GetProjectionViewMatix());
	}

	void Renderer2D::LineEndScene()
	{
		m_data->Linevb->SetData(sizeof(LineAttributes) * 2 * m_data->m_LineVertCounter, &m_data->Line[0].Position);
		uint32_t x = (2 * m_data->m_LineVertCounter) -1;
		RenderCommand::DrawLine(*m_data->Linevao, x);
		m_data->m_LineVertCounter = 0;
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

		if(tex)
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
	void Renderer2D::DrawLine(const glm::vec3& p1,const glm::vec3& p2 , const glm::vec4& color, const float& width)
	{
		//if (m_data->m_LineVertCounter >= m_data->Line.size())
		//{
		//	LineEndScene();//if ran out of cache then partiatilly render the cache
		//	Init();
		//	return;
		//}
		m_data->Line[m_data->m_LineVertCounter + 0] = LineAttributes(glm::vec4(p1, 1), color);
		m_data->Line[m_data->m_LineVertCounter + 1] = LineAttributes(glm::vec4(p2, 1), color);
		m_data->m_LineVertCounter += 2;
		glLineWidth(width);
		//glColor4f(0, 1, 0, 1);
	}
	void Renderer2D::DrawCurve(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& v0, const glm::vec2& v1 , const glm::vec4& color)
	{
		//hermite curve implementation best for joining points in a smooth manner
		glm::vec2 p;
		glm::vec3 tmp = glm::vec3(p0, 0.0f);

		for (int i = 1; i <= 10; i++)//iterating the value of t
		{
			double t = ( double)i / 10.0;
			p.x = p0.x +
				t * v0.x +
				t * t * (-3 * p0.x - 2 * v0.x + 3 * p1.x - v1.x) +
				t * t * t * (2 * p0.x + v0.x - 2 * p1.x + v1.x);
			p.y = p0.y +
				t * v0.y +
				t * t * (-3 * p0.y - 2 * v0.y + 3 * p1.y - v1.y) +
				t * t * t * (2 * p0.y + v0.y - 2 * p1.y + v1.y);
			Renderer2D::DrawLine(tmp, glm::vec3(p, 0.0f), color);
			tmp = glm::vec3(p, 0);
		}
		
	}
	void Renderer2D::Draw_Bezier_Curve(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3)
	{

		glm::vec2 p;
		glm::vec3 tmp = glm::vec3(p0, 0);

		for (int i = 1; i <= 10; i++)//iterating the value of t
		{
			float t = (float)i / 10.0;
			p.x = p0.x +
				t * (-3 * p0.x + 3 * p1.x) +
				t * t * (3 * p0.x - 6 * p1.x + 3 * p2.x) +
				t * t * t * (-p0.x + 3 * p1.x - 3 * p2.x + p3.x);
			p.y = p0.y +
				t * (-3 * p0.y + 3 * p1.y) +
				t * t * (3 * p0.y - 6 * p1.y + 3 * p2.y) +
				t * t * t * (-p0.y + 3 * p1.y - 3 * p2.y + p3.y);
			Renderer2D::DrawLine(tmp, glm::vec3(p, 0), { 0.9,0.4,0.6,1 });
			tmp = glm::vec3(p, 0);
		}
	}
}