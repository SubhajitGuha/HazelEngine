#include "hzpch.h"
#include "RayTracer.h"
#include "glad/glad.h"

namespace Hazel
{
	uint32_t RayTracer::m_RT_TextureID;
	RayTracer::RayTracer()
	{
		image_width = 512;
		image_height = 512;
		viewport_height = 2.0f;
		viewport_width = viewport_height * float(image_width)/image_height;
		samples = 2;
		Init();
	}
	RayTracer::RayTracer(int image_w, int image_h, int viewport_w, int viewport_h, int samples)
	{
		image_width = image_w;
		image_height = image_h;
		//viewport_width = viewport_w;
		//viewport_height = viewport_h;
		this->samples = samples;
		Init();
	}
	void RayTracer::Init()
	{
		m_Binding = 1;
		m_focalLength = 10.f;
		glGenTextures(1, &m_RT_TextureID);
		glBindTexture(GL_TEXTURE_2D, m_RT_TextureID);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, image_width, image_height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindImageTexture(m_Binding, m_RT_TextureID, 0, 0, 0, GL_READ_WRITE, GL_RGBA8);
		cs_RayTracingShader = Shader::Create("Assets/Shaders/cs_RayTracingShader.glsl");
	}
	void RayTracer::RenderImage(Camera& cam)
	{
		viewport_height = 2.0 * m_focalLength * glm::tan(glm::radians(cam.GetVerticalFOV())/2.0f); // 10.0f is the focal length of the camera get height from Vfov
		viewport_width = viewport_height * float(image_width) / image_height;
		cs_RayTracingShader->Bind();
		cs_RayTracingShader->SetFloat("viewport_w",viewport_width);
		cs_RayTracingShader->SetFloat("viewport_h", viewport_height);
		cs_RayTracingShader->SetFloat3("camera_pos", cam.GetCameraPosition());
		cs_RayTracingShader->SetFloat3("camera_viewdir", -glm::normalize(cam.GetViewDirection()));
		cs_RayTracingShader->SetFloat("focal_length", m_focalLength);
		glDispatchCompute(image_width/32, image_height/32, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}
}