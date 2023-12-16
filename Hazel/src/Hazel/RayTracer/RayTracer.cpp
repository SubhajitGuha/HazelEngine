#include "hzpch.h"
#include "RayTracer.h"
#include "glad/glad.h"

namespace Hazel
{
	uint32_t RayTracer::m_RT_TextureID;
	RayTracer::RayTracer()
	{
		//image_width = 512;
		//image_height = 512;
		viewport_height = 2.0f;
		viewport_width = viewport_height * float(image_width)/image_height;
		samples = 2;
		Init(512,512);
		StartTime = std::chrono::high_resolution_clock::now();
	}
	RayTracer::RayTracer(int image_w, int image_h, int viewport_w, int viewport_h, int samples)
	{		
		//viewport_width = viewport_w;
		//viewport_height = viewport_h;
		this->samples = samples;
		Init(image_w, image_h);
	}
	void RayTracer::Init(int width, int height)
	{
		m_Binding = 1;
		m_focalLength = 10.f;
		image_width = width;
		image_height = height;
		glGenTextures(1, &m_RT_TextureID);
		glBindTexture(GL_TEXTURE_2D, m_RT_TextureID);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, image_width, image_height);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindImageTexture(m_Binding, m_RT_TextureID, 0, 0, 0, GL_READ_WRITE, GL_RGBA8);
		cs_RayTracingShader = Shader::Create("Assets/Shaders/cs_RayTracingShader.glsl");
	}
	void RayTracer::RenderImage(Camera& cam)
	{
		float time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-StartTime).count();
		cs_RayTracingShader->Bind();
		cs_RayTracingShader->SetFloat("viewport_w",viewport_width);
		cs_RayTracingShader->SetFloat("viewport_h", viewport_height);
		cs_RayTracingShader->SetFloat("time", time);
		cs_RayTracingShader->SetFloat3("camera_pos", cam.GetCameraPosition());
		cs_RayTracingShader->SetFloat3("camera_viewdir", -cam.GetViewDirection());
		cs_RayTracingShader->SetFloat("focal_length", m_focalLength);
		cs_RayTracingShader->SetMat4("mat_view", cam.GetViewMatrix());
		cs_RayTracingShader->SetMat4("mat_proj", cam.GetProjectionMatrix());
		cs_RayTracingShader->SetFloat3("light_dir", glm::normalize(Renderer3D::m_SunLightDir));
		
		glDispatchCompute(image_width/8, image_height/8, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}
	void RayTracer::Resize(int width, int height)
	{
		m_RT_TextureID = 0;
		Init(width,height);
	}
}