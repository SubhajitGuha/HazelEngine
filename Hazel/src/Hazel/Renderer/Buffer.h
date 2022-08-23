#pragma once
#include "Hazel/Log.h"

//This Buffer file contains Both index and vertex Buffer oi
namespace Hazel {


	enum class DataType {
		None=0,Float,Float2,Float3,Float4,Int,Int2,Int3,Int4
	};


	struct BufferElements {
		std::string Name;
		DataType Type;
		size_t Offset;
		bool Normalized;
		BufferElements(std::string name,DataType type)
			:Type(type),Name(name),Offset(0),Normalized(false){}
	};


	class BufferLayout {
	public:
		BufferLayout() = default;
		void push(std::string name, DataType type);

		inline std::vector<BufferElements*> GetElements() { return m_Elements; }
		inline unsigned int GetStride() { return Stride; }
	
	private:
		unsigned int GetSize(DataType t);
		std::vector<BufferElements*> m_Elements;
		unsigned int Stride=0;
	};


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


	class VertexArray {
	public:
		virtual void Bind()const = 0;
		virtual void UnBind()const = 0;
		virtual void AddBuffer(BufferLayout&, VertexBuffer&){}
		static VertexArray* Create();
	};

}