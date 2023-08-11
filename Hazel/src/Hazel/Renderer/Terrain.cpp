#include "hzpch.h"
#include "Hazel/Log.h"
#include "Terrain.h"
#include "glad/glad.h"
#include "stb_image.h"
namespace Hazel
{
	Terrain::Terrain(float width, float height)
	{
		m_dimension.x = width;
		m_dimension.y = height;
		m_maxTerrainHeight = std::numeric_limits<float>::min();
		m_terrainShader = Shader::Create("Assets/Shaders/TerrainShader.glsl");
		InitilizeTerrain();
	}
	Terrain::~Terrain()
	{
		
	}
	void Terrain::InitilizeTerrain()
	{
		unsigned char* data = stbi_load("Assets/Textures/Terrain_Height_Map.png",&m_Width,&m_Height,&m_Channels,0);
		
		m_HeightMap = Texture2D::Create("Assets/Textures/Terrain_Height_Map.png");
		m_terrainShader->Bind();
		m_HeightMap->Bind(HEIGHT_MAP_TEXTURE_SLOT);
		m_terrainShader->SetInt("u_HeightMap", HEIGHT_MAP_TEXTURE_SLOT);

		m_terrainVertexArray = VertexArray::Create();

		for (int j = 0; j < m_dimension.x; j++) {
			for (int i = 0; i < m_dimension.y * m_Channels; i += m_Channels)
			{
				int y = data[j * m_Width + i];//R channel of 1st vertex

				if (m_maxTerrainHeight < y)
					m_maxTerrainHeight = y;
			}
		}
		//m_dimension is <= m_width,m_height
		for (int j = 0; j < m_dimension.x ; j++) {
			for (int i = 0; i < m_dimension.y ; i++) 
			{
				int y = data[(j * m_Width + i)*m_Channels];//R channel vertex

				glm::vec3 pos = { i ,(y/m_maxTerrainHeight)*500,j};
				glm::vec2 tcoord = { i / m_dimension.y,j / m_dimension.x };
				glm::vec3 normal = { 0,0,0 };
				terrainData.push_back({ pos,tcoord,normal });
			}
		}

		std::vector<unsigned int> index_buf(m_dimension.x * m_dimension.y);
		int offset = 0;
		for (int i = 0; i <= m_dimension.x * m_dimension.y; i += 6)
		{
			//do normal calculation here
			index_buf[i + 0] = offset;
			index_buf[i + 1] = (offset + m_dimension.x);
			index_buf[i + 2] = (offset + m_dimension.x + 1);
			index_buf[i + 3] = offset;
			index_buf[i + 4] = offset + m_dimension.x + 1;
			index_buf[i + 5] = offset + 1;

			auto& v1 = terrainData[index_buf[i]];
			auto& v2 = terrainData[index_buf[i + 1]];
			auto& v3 = terrainData[index_buf[i + 2]];
			auto& v4 = terrainData[index_buf[i + 3]];
			auto& v5 = terrainData[index_buf[i + 4]];
			auto& v6 = terrainData[index_buf[i + 5]];

			//normal for v1
			v1.Normal = glm::cross(v2.Position - v1.Position, v3.Position- v1.Position);
			//normal for v2
			v2.Normal = glm::cross(v1.Position - v2.Position, v3.Position - v2.Position);
			//normal for v3
			v3.Normal = glm::cross(v2.Position - v3.Position, v1.Position - v3.Position);
			//normal for v4
			v3.Normal = glm::cross(v5.Position - v4.Position, v6.Position - v4.Position);
			//normal for v5
			v3.Normal = glm::cross(v4.Position - v5.Position, v6.Position - v5.Position);
			//normal for v6
			v3.Normal = glm::cross(v5.Position - v6.Position, v4.Position - v6.Position);
			offset += 1;
		}

		ref<VertexBuffer> vb = VertexBuffer::Create(&terrainData[0].Position.x, sizeof(TerrainData)*terrainData.size());
		bl = std::make_shared<BufferLayout>();
		bl->push("Position", DataType::Float3);
		bl->push("coord", DataType::Float2);
		bl->push("normal", DataType::Float3);
		ref<IndexBuffer> ib = IndexBuffer::Create(&index_buf[0], sizeof(unsigned int) * index_buf.size());
		m_terrainVertexArray->AddBuffer(bl, vb);
		m_terrainVertexArray->SetIndexBuffer(ib);

		stbi_image_free(data);
	}
	void Terrain::RenderTerrain(Camera& cam)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), { 0,1,0 }) * glm::rotate(glm::mat4(1.0), glm::radians(180.0f), {0,0,1});
		m_terrainShader->Bind();
		m_terrainShader->SetMat4("u_ProjectionView", cam.GetProjectionView());
		m_terrainShader->SetMat4("u_Model", transform);
		m_terrainShader->SetFloat("u_maxTerrainHeight", m_maxTerrainHeight);
		//m_terrainShader->SetMat4("u_Model", transform);

		//m_terrainVertexArray->Bind();
		//glDrawArrays(GL_LINES, 0, terrainData.size());
		//RenderCommand::DrawArrays(*m_terrainVertexArray, terrainData.size());
		RenderCommand::DrawIndex(*m_terrainVertexArray);
	}
}