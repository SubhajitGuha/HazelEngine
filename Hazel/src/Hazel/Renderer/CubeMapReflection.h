#pragma once
#include "Hazel.h"

namespace Hazel {
	class CubeMapReflection
	{
	public:
		CubeMapReflection();
		~CubeMapReflection();
		virtual void RenderToCubeMap(Scene& scene) = 0;
		virtual void Bind(int slot) = 0;
		virtual void UnBind() = 0;
		virtual unsigned int GetTexture_ID() = 0;
		virtual void SetCubeMapResolution(float width, float height) = 0;
		static ref<CubeMapReflection> Create();
	};
}