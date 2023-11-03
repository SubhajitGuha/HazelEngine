#pragma once
#include "Hazel.h"

namespace Hazel
{
	class Fog
	{
	public:
		static ref<Fog> Create(float density, float gradient, float fogStart, float fogEnd, glm::vec2 ScreenSize = { 1024,1024 });

		virtual void SetFogParameters(float density, float gradient, glm::vec3 fogColor) = 0;
		virtual void RenderFog(Camera& cam, glm::vec2 screenSize) = 0;
	};
}