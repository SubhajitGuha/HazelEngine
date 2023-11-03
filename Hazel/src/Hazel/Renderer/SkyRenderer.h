#pragma once
#include "Hazel.h"

namespace Hazel
{
	enum SkyType
	{
		CUBE_MAP_SKY,PROCEDURAL_SKY
	};
	class SkyRenderer {
	public:
		static void SetSkyType(SkyType type) { m_skyType = type; }
		//use path if the sky type is a cube map
		static void Initilize(const std::string& path = "");

		static void RenderSky(Camera& camera);
	private:
		static ref<SkyRenderer> m_skyRenderer;
		static SkyType m_skyType;
	};
}