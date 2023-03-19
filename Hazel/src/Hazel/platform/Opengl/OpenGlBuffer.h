#pragma once
#include "Hazel/Renderer/Buffer.h"
namespace Hazel {

	class OpenGlVertexBuffer :public VertexBuffer {
	public:
		OpenGlVertexBuffer(float* data, size_t size);
		OpenGlVertexBuffer(size_t size, BufferStorage_Type Storage_Type = MUTABLE);
		~OpenGlVertexBuffer();
		virtual void Bind() const override;
		virtual void UnBind() const override;
		virtual void SetData(size_t size, const void* data) override;
		virtual void* MapBuffer(size_t size) override;
	private:
			unsigned int m_RendererID;
	};


	class OpenGlIndexBuffer :public IndexBuffer {
	public:
		OpenGlIndexBuffer(unsigned int* data, size_t size);
		~OpenGlIndexBuffer();
		void Bind() const override;
		void UnBind() const override;
		size_t GetCount()override;
	private:
		size_t m_Elements;
		unsigned int m_RendererID;
	};
}