#include "hzpch.h"
#include "RayTracer.h"
#include "glad/glad.h"

namespace Hazel
{
	uint32_t RayTracer::m_RT_TextureID;

	glm::vec4 RayTracer::m_Color = glm::vec4(1.0,0.6,0.1,1.0);
	float RayTracer::m_Roughness = 1.0f;
	bool RayTracer::EnableSky = true;
	int RayTracer::numBounces = 10;
	int RayTracer::samplesPerPixel = 2;

	RayTracer::RayTracer()
	{
		frame_num = 1;
		samples = 2;
		Init(512,512);
		bvh = std::make_shared<BVH>();
		StartTime = std::chrono::high_resolution_clock::now();

		//pass the nodes,triangles as ssbos
		glGenBuffers(1, &ssbo_linearBVHNodes);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_linearBVHNodes);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(BVH::LinearBVHNode) * bvh->arrLinearBVHNode.size(), &bvh->arrLinearBVHNode[0], GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_linearBVHNodes);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glGenBuffers(1, &ssbo_rtTriangles);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_rtTriangles);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(BVH::RTTriangles) * bvh->arrRTTriangles.size(), &bvh->arrRTTriangles[0], GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo_rtTriangles);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glGenBuffers(1, &ssbo_triangleIndices);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_triangleIndices);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * bvh->triIndex.size(), &bvh->triIndex[0], GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo_triangleIndices);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
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
		bvh->texArray->Bind(ALBEDO_SLOT);
		cs_RayTracingShader->SetInt("albedo", ALBEDO_SLOT);
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
		cs_RayTracingShader->SetInt("BVHNodeSize", bvh->arrLinearBVHNode.size());
		cs_RayTracingShader->SetFloat("light_intensity", Renderer3D::m_SunIntensity);
		cs_RayTracingShader->SetFloat4("u_Color", m_Color);
		cs_RayTracingShader->SetFloat("u_Roughness", m_Roughness);

		//bind the ssbo objects
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_linearBVHNodes);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_linearBVHNodes);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_rtTriangles);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo_rtTriangles);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_triangleIndices);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo_triangleIndices);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

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
		cs_RayTracingShader->SetFloat4("u_Color", m_Color);
		cs_RayTracingShader->SetFloat("u_Roughness", m_Roughness);
	}
}