#pragma once
#include "Hazel/Log.h"
#include "Hazel/Renderer/Buffer.h"

namespace Hazel {
	class OpenGlVertexArray : public VertexArray
	{
	public:
		OpenGlVertexArray();
		~OpenGlVertexArray();
		void Bind()const override;
		void UnBind()const override;
		void AddBuffer(std::shared_ptr<BufferLayout>& layout, std::shared_ptr<VertexBuffer>& vbo) override;
		void SetIndexBuffer(std::shared_ptr<IndexBuffer> IndexBuffer)override;

		const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }
		const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() override { return m_VertexBuffer; }
		unsigned int GetVertexArrayID() override { return m_Renderer; }
	private:
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffer;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
		unsigned int m_Renderer;
	};
}

