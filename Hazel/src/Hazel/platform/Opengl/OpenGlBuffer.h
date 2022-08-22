#pragma once
#include "Hazel/Renderer/Buffer.h"
namespace Hazel {

	class OpenGlVertexBuffer :public VertexBuffer {
	public:
		OpenGlVertexBuffer(float* data, size_t size);
		~OpenGlVertexBuffer();
		void Bind() const override;
		void UnBind() const override;
	private:
			unsigned int m_Renderer;
	};


	class OpenGlIndexBuffer :public IndexBuffer {
	public:
		OpenGlIndexBuffer(unsigned int* data, size_t size);
		~OpenGlIndexBuffer();
		void Bind() const override;
		void UnBind() const override;
	private:
		unsigned int m_Renderer;
	};
}