#pragma once
#include "Camera.h"

namespace Hazel {
	class SceneCamera :public Camera {
	public:
		SceneCamera(float Width, float Height);
		SceneCamera();
		~SceneCamera() = default;
		void SetViewportSize(float, float);
		void SetOrthographic(float Size, float Near = -1.0f, float Far = 1.0f);
		void RecalculateProjection();//for now it is orthographic projection 
	private:
		float m_OrthographicSize = 10.f;
		float m_OrthographicFar = 1.f, m_OrthographicNear = -1.f;
		float m_AspectRatio=1.0f;
	};
}