
#include "hzpch.h"
#include "SkyRenderer.h"
#include "Atmosphere.h"

namespace Hazel {
	ref<SkyRenderer> SkyRenderer::m_skyRenderer = nullptr;
	SkyType SkyRenderer::m_skyType = SkyType::CUBE_MAP_SKY;

	void SkyRenderer::Initilize()
	{
		CubeMapEnvironment::Init();
		Atmosphere::InitilizeAtmosphere();
	}

	void SkyRenderer::RenderSky(Camera& camera)
	{
		switch (m_skyType)
		{
		case SkyType::CUBE_MAP_SKY:
			CubeMapEnvironment::RenderCubeMap(camera.GetViewMatrix(),camera.GetProjectionMatrix(),camera.GetViewDirection());
			break;
		case SkyType::PROCEDURAL_SKY:
			Atmosphere::RenderAtmosphere(camera);
			break;
		default:
			return;
		}
	}
}