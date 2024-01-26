#include "hzpch.h"
#include "RayTracer.h"
#include "glad/glad.h"

namespace Hazel
{
	uint32_t RayTracer::m_Sampled_TextureID; bool RayTracer::isViewportFocused = false;

	glm::vec3 RayTracer::m_LightPos = glm::vec3(-122.0,46.83,-55.0);
	float RayTracer::m_LightStrength = 20.0f;
	bool RayTracer::EnableSky = true;
	int RayTracer::numBounces = 2;
	int RayTracer::samplesPerPixel = 1;

	RayTracer::RayTracer()
	{
		frame_num = 1;
		sample_count = 1;
		samples = 1;
		tile_size = { 128,128 };
		tile_index = { 0,0 };
		Init(512,512);
		bvh = std::make_shared<BVH>(Scene::Sphere);
		StartTime = std::chrono::high_resolution_clock::now();

		glGenFramebuffers(1, &m_fbo); //create framebuffer object to copy the final rendered image
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		GLenum buffers[1] = { GL_COLOR_ATTACHMENT0};
		glDrawBuffers(1, buffers);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//create buffers to pass the linear-bvh
		//pass the nodes,triangles as ssbos
		glGenBuffers(1, &ssbo_linearBVHNodes);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_linearBVHNodes);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(BVH::LinearBVHNode) * bvh->arrLinearBVHNode.size(), &bvh->arrLinearBVHNode[0], GL_STATIC_READ);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_linearBVHNodes);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glGenBuffers(1, &ssbo_rtTriangles);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_rtTriangles);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(BVH::RTTriangles) * bvh->arrRTTriangles.size(), &bvh->arrRTTriangles[0], GL_STATIC_READ);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo_rtTriangles);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glGenBuffers(1, &ssbo_triangleIndices);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_triangleIndices);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * bvh->triIndex.size(), &bvh->triIndex[0], GL_STATIC_READ);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo_triangleIndices);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		//create buffers to pass the materials
		//glGenBuffers(1, &ssbo_arrMaterials);
		//glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_arrMaterials);
		//glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(BVH::Material) * bvh->arrMaterials.size(), &bvh->arrMaterials[0], GL_STATIC_READ);
		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssbo_arrMaterials);
		//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		cs_RayTracingShader = Shader::Create("Assets/Shaders/cs_RayTracingShader.glsl");
		RayTracing_CopyShader = Shader::Create("Assets/Shaders/RayTracing_CopyShader.glsl");
	}
	
	RayTracer::RayTracer(int image_w, int image_h, int viewport_w, int viewport_h, int samples)
	{		
		this->samples = samples;
		Init(image_w, image_h);
	}
	void RayTracer::Init(int width, int height)
	{
		m_focalLength = 10.f;
		image_width = width;
		image_height = height;
		
		if (m_LowRes_TextureID == 0) 
		{
			glGenTextures(1, &m_LowRes_TextureID);
			glBindTexture(GL_TEXTURE_2D, m_LowRes_TextureID);
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16, tile_size.x, tile_size.y);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glGenTextures(1, &m_Sampled_TextureID);
		glBindTexture(GL_TEXTURE_2D, m_Sampled_TextureID);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16, image_width, image_height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &m_RT_TextureID);
		glBindTexture(GL_TEXTURE_2D, m_RT_TextureID);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16, image_width, image_height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);		
	}

	void RayTracer::draw(Camera& cam, uint32_t& outputTextureID)
	{
		cs_RayTracingShader->Bind();
		//cs_RayTracingShader->SetFloat("time", time);
		//bvh->texArray_albedo->Bind(ALBEDO_SLOT);
		//bvh->texArray_roughness->Bind(ROUGHNESS_SLOT);

		cs_RayTracingShader->SetInt("albedo", ALBEDO_SLOT);
		cs_RayTracingShader->SetInt("roughness_metallic", ROUGHNESS_SLOT);
		cs_RayTracingShader->SetFloat3("camera_pos", cam.GetCameraPosition());
		cs_RayTracingShader->SetFloat3("camera_viewdir", -cam.GetViewDirection());
		cs_RayTracingShader->SetFloat("focal_length", m_focalLength);
		cs_RayTracingShader->SetMat4("mat_view", cam.GetViewMatrix());
		cs_RayTracingShader->SetMat4("mat_proj", cam.GetProjectionMatrix());
		cs_RayTracingShader->SetFloat3("light_dir", glm::normalize(Renderer3D::m_SunLightDir));
		cs_RayTracingShader->SetInt("EnvironmentEnabled", EnableSky);
		cs_RayTracingShader->SetInt("frame_num", abs(frame_num));
		cs_RayTracingShader->SetInt("sample_count", abs(sample_count));
		cs_RayTracingShader->SetInt("num_bounces", numBounces);
		cs_RayTracingShader->SetInt("samplesPerPixel", samplesPerPixel);
		cs_RayTracingShader->SetInt("BVHNodeSize", bvh->arrLinearBVHNode.size());
		cs_RayTracingShader->SetFloat("light_intensity", Renderer3D::m_SunIntensity);
		cs_RayTracingShader->SetFloat3("LightPos", m_LightPos);
		cs_RayTracingShader->SetFloat("u_LightStrength", m_LightStrength);
		cs_RayTracingShader->SetFloat("u_ImageWidth", image_width);
		cs_RayTracingShader->SetFloat("u_ImageHeight", image_height);

		cs_RayTracingShader->SetInt("TileIndex_X", tile_index.x);
		cs_RayTracingShader->SetInt("TileIndex_Y", tile_index.y);

		//bvh->UpdateMaterials(); //update material every frame

		glBindImageTexture(1, outputTextureID, 0, 0, 0, GL_READ_WRITE, GL_RGBA16);
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

		//pass material data to the gpu using ssbo every frame
		glGenBuffers(1, &ssbo_arrMaterials);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_arrMaterials);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(BVH::Material) * bvh->arrMaterials.size(), &bvh->arrMaterials[0], GL_STATIC_READ);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssbo_arrMaterials);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glDispatchCompute(tile_size.x / 4, tile_size.y / 4, 1); //render by tile size
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

	}

	void RayTracer::RenderLowResImage(Camera& cam)
	{
		int w = image_width;
		int h = image_height;
		image_width = tile_size.x;
		image_height = tile_size.y;
		draw(cam, m_LowRes_TextureID);
		image_width = w;
		image_height = h;
	}

	void RayTracer::copyAccmulatedImage(uint32_t& ImageToCopy)
	{
		glBindTextureUnit(PT_IMAGE_SLOT, ImageToCopy);
		RayTracing_CopyShader->Bind();
		RayTracing_CopyShader->SetInt("InputTexture", PT_IMAGE_SLOT);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		glViewport(0, 0, image_width, image_height);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Sampled_TextureID, 0);
		RenderScreenSizeQuad();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void RayTracer::RenderImage(Camera& cam)
	{
		//if any if the buttons are pressed then re-initilize the frame
		if (isViewportFocused && (Input::IsButtonPressed(HZ_MOUSE_BUTTON_1) || Input::IsButtonPressed(HZ_MOUSE_BUTTON_2) || 
			Input::IsKeyPressed(HZ_KEY_W) || Input::IsKeyPressed(HZ_KEY_A) || Input::IsKeyPressed(HZ_KEY_S)||
			Input::IsKeyPressed(HZ_KEY_D) || Input::IsKeyPressed(HZ_KEY_Q) || Input::IsKeyPressed(HZ_KEY_E)))
		{
			isMoved = true;
		}

		//float time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-StartTime).count();
		if (isMoved)
		{
			RenderLowResImage(cam);
			copyAccmulatedImage(m_LowRes_TextureID); //copy the low-res image if camera is moved or viewport is resized
			isMoved = false;
			frame_num = 1; //reset frame counter
			sample_count = 1; //reset sample counter
			tile_index = { 0,0 }; //reset tile indices
		}
		else 
		{
			draw(cam, m_RT_TextureID);

			frame_num++;
			//increment tile indices
			tile_index.x++;
			if (tile_index.x > image_width / tile_size.x)
			{
				tile_index.x = 0;
				tile_index.y++;
				if (tile_index.y > image_height / tile_size.y)
				{
					sample_count++;
					tile_index = { 0,0 };
					//copy the final rendered image
					copyAccmulatedImage(m_RT_TextureID);
				}
			}
		}
	}
	void RayTracer::Resize(int width, int height)
	{
		isMoved = true;
		Init(width,height);
	}
	void RayTracer::UpdateScene()
	{
		//cs_RayTracingShader->SetFloat3("LightPos", m_LightPos);
		//cs_RayTracingShader->SetFloat("u_Roughness", m_Roughness);
	}
	void RayTracer::RenderScreenSizeQuad()
	{
		//this function renders a quad infront of the camera
		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);//disable depth testing

		glm::vec4 data[] = {
		glm::vec4(-1,-1,0,1),glm::vec4(0,0,0,0),
		glm::vec4(1,-1,0,1),glm::vec4(1,0,0,0),
		glm::vec4(1,1,0,1),glm::vec4(1,1,0,0),
		glm::vec4(-1,1,0,1),glm::vec4(0,1,0,0)
		};

		ref<VertexArray> vao = VertexArray::Create();
		ref<VertexBuffer> vb = VertexBuffer::Create(&data[0].x, sizeof(data));
		unsigned int i_data[] = { 0,1,2,0,2,3 };
		ref<IndexBuffer> ib = IndexBuffer::Create(i_data, sizeof(i_data));

		ref<BufferLayout> bl = std::make_shared<BufferLayout>(); //buffer layout

		bl->push("position", DataType::Float4);
		bl->push("direction", DataType::Float4);

		vao->AddBuffer(bl, vb);
		vao->SetIndexBuffer(ib);

		RenderCommand::DrawIndex(*vao);

		glDepthMask(GL_TRUE);//again enable depth testing
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
}