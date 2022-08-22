#pragma once
#include "Hazel/Log.h"

//This Buffer file contains Both index and vertex Buffer oi
namespace Hazel {
	class VertexBuffer {
	public:
		virtual void Bind()const = 0;
		virtual void UnBind()const = 0;

		static VertexBuffer* Create(float* data, size_t size);
	};
	class IndexBuffer {
	public:
		virtual void Bind()const = 0;
		virtual void UnBind()const = 0;
		static IndexBuffer* Create(unsigned int* data, size_t size);
	};
}