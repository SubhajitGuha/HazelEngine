#include "hzpch.h"
#include "RayTracer.h"
#include "glad/glad.h"

namespace Hazel
{
	uint32_t RayTracer::m_RT_TextureID;
	std::vector<glm::vec3> RayTracer::m_SpherePos;
	std::vector<float> RayTracer::m_SphereRadius;

	std::vector<glm::vec4> RayTracer::m_SphereCol;
	std::vector<glm::vec4> RayTracer::m_SphereEmissionCol;
	std::vector<float> RayTracer::m_SphereEmissionStrength;
	std::vector<float> RayTracer::m_SphereRoughness;
	bool RayTracer::EnableSky = true;
	int RayTracer::numBounces = 30;
	int RayTracer::samplesPerPixel = 20;

	RayTracer::RayTracer()
	{
		frame_num = 1;
		samples = 2;
		Init(512,512);
		bvh = std::make_shared<BVH>();
		StartTime = std::chrono::high_resolution_clock::now();
	}
	RayTracer::RayTracer(int image_w, int image_h, int viewport_w, int viewport_h, int samples)
	{		
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
		if (old_view != cam.GetViewMatrix())
		{
			old_view = cam.GetViewMatrix();
			frame_num = 1;
		}
		//float time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-StartTime).count();
		cs_RayTracingShader->Bind();
		//cs_RayTracingShader->SetFloat("time", time);
		cs_RayTracingShader->SetFloat3("camera_pos", cam.GetCameraPosition());
		cs_RayTracingShader->SetFloat3("camera_viewdir", -cam.GetViewDirection());
		cs_RayTracingShader->SetFloat("focal_length", m_focalLength);
		cs_RayTracingShader->SetMat4("mat_view", cam.GetViewMatrix());
		cs_RayTracingShader->SetMat4("mat_proj", cam.GetProjectionMatrix());
		cs_RayTracingShader->SetFloat3("light_dir", glm::normalize(Renderer3D::m_SunLightDir));
		cs_RayTracingShader->SetInt("EnvironmentEnabled", EnableSky);
		cs_RayTracingShader->SetInt("frame_num", abs(frame_num));
		cs_RayTracingShader->SetInt("num_bounces", numBounces);
		cs_RayTracingShader->SetInt("samplesPerPixel", samplesPerPixel);

		//UpdateScene();
		glDispatchCompute(image_width/8, image_height/8, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		frame_num++;
		frame_num %= 2147483647;
	}
	void RayTracer::Resize(int width, int height)
	{
		frame_num = 1;
		m_RT_TextureID = 0;
		Init(width,height);
	}
	void RayTracer::UpdateScene()
	{
		int num_spheres = m_SpherePos.size();
		cs_RayTracingShader->SetInt("num_spheres", num_spheres);
		cs_RayTracingShader->SetFloat3Array("SpherePos", &m_SpherePos[0].x, num_spheres);
		cs_RayTracingShader->SetFloatArray("SphereRadius", m_SphereRadius[0], num_spheres);
		cs_RayTracingShader->SetFloat4Array("SphereCol", &m_SphereCol[0].x, num_spheres);
		cs_RayTracingShader->SetFloat4Array("SphereEmissionCol", &m_SphereEmissionCol[0].x, num_spheres);
		cs_RayTracingShader->SetFloatArray("SphereEmissionStrength", m_SphereEmissionStrength[0], num_spheres);
		cs_RayTracingShader->SetFloatArray("SphereRoughness", m_SphereRoughness[0], num_spheres);
	}
}