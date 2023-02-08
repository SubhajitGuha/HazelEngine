#pragma once
#include "Hazel/Core.h"

namespace Hazel {
	class Texture
	{
	public:
		virtual unsigned int GetWidth() = 0;
		virtual unsigned int GetHeight() = 0;
		virtual unsigned int GetID() = 0;
		virtual void Bind(int slot)const=0;
	};
	class Texture2D :public Texture {
	public:

		static bool ValidateTexture(const std::string& path);
		virtual void UnBind()const = 0;
		static ref<Texture2D> Create(const std::string& path);
		static ref<Texture2D> Create(const unsigned int Width,const unsigned int Height, unsigned int);
	};
}

