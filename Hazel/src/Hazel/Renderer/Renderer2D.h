#pragma once
#include "Hazel/Core.h"
#include "OrthographicCamera.h"
#include "Hazel/Renderer/Camareas/Camera.h"
#include "glm/glm.hpp"
#include "Shader.h"
#include "Texture.h"
#include "SubTexture.h"


namespace Hazel {
	class Camera;
	class Renderer2D {
	private:
		static void StartBatch();
		static void Flush();
		static void FlushLine();
		static void NextBatch();
	public:
		static void Init();
		static void BeginScene(OrthographicCamera& camera);
		//static void BeginScene(Camera& camera);
		static void BeginScene(Camera&);
		static void EndScene();
		static void LineBeginScene(OrthographicCamera& camera);
		static void LineBeginScene(Camera& camera);
		static void LineEndScene();

	public:
		static void DrawQuad(const glm::vec3& pos, const glm::vec3& scale, const glm::vec4& col, const float angle = 0.f);
		static void DrawQuad(const glm::vec3& pos, const glm::vec3& scale, ref<Texture2D> tex, unsigned int index = 1, const float angle = 0.f);
		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, ref<Texture2D> texture=nullptr);
		static void DrawSubTexturedQuad(const glm::vec3& pos, const glm::vec3& scale, ref<SubTexture2D> tex, unsigned int index = 2, const float angle = 0.f);
		static void DrawLine(const glm::vec3& p1,const glm::vec3& p2,const glm::vec4& color,const float& width = 4.0);
		static void DrawCurve(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& v0, const glm::vec2& v1, const glm::vec4& color = {0.5,0.8,0.1,1},float width = 4.0f);
		static void Draw_Bezier_Curve(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3);

		//note texture coord defines the uv coordinate of the texture (in sprite sheet we need these texture coordinates to get individual asset images)
	};
}