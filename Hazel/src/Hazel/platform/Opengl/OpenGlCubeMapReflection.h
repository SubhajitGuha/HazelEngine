#pragma once
#include "Hazel.h"
#include "Hazel/Renderer/CubeMapReflection.h"

namespace Hazel {
	class OpenGlCubeMapReflection:public CubeMapReflection
	{
	public:
		OpenGlCubeMapReflection();
		~OpenGlCubeMapReflection();
		void CreateCubeMapTexture();
		virtual void RenderToCubeMap(Scene& scene) override;
		virtual void Bind(int slot) override;
		virtual void UnBind() override;
		virtual unsigned int GetTexture_ID() override;
		virtual void SetCubeMapResolution(float width, float height) override;
		void SwitchToFace(int n);

	private:
		unsigned int tex_id, framebuffer_id,depth_id;
		ref<Shader> shader;
		float cubemap_width=2048, cubemap_height=2048;
		int slot = 10;
		float yaw = 0, pitch = 0;
	};
}