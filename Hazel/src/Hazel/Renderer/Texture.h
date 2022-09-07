#pragma once
#include "Hazel/Core.h"
namespace Hazel {
	class Texture
	{
	public:
		virtual unsigned int GetWidth() = 0;
		virtual unsigned int GetHeight() = 0;

		virtual void Bind(int slot)const=0;
	};
	class Texture2D :public Texture {
	public:

		virtual void UnBind()const = 0;
		static ref<Texture> Create(const std::string& path);
	};
}

