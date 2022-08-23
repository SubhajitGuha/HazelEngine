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
		void AddBuffer(BufferLayout& layout, VertexBuffer& vbo) override;
	private:
		unsigned int m_Renderer;
	};
}

