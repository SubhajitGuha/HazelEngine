#pragma once
#include "Camera.h"

namespace Hazel {
	enum ProjectionTypes {
		Orhtographic=0, perspective=1
	};

	class SceneCamera :public Camera {
	public:
		SceneCamera(float Width, float Height);
		SceneCamera();
		~SceneCamera() = default;

		void SetProjectionType(ProjectionTypes type) { m_projection_type = type; RecalculateProjection(); }
		void SetViewportSize(float, float);
		void SetPerspective(float verticalFOV, float Near, float Far);
		void SetOrthographic(float Size, float Near = -1.0f, float Far = 1.0f);

		inline float GetOrthographicSize() { return m_OrthographicSize; }
		inline float GetOrthographicNear() { return m_OrthographicNear; }
		inline float GetOrthographicFar() { return m_OrthographicFar; }
		void SetOrthographicSize(float size) { m_OrthographicSize = size; }
		void SetOrthographicNear(float Near) { m_OrthographicNear = Near; RecalculateProjection(); }
		void SetOrthographicFar(float Far) { m_OrthographicFar = Far; RecalculateProjection(); }

		inline float GetPerspectiveVerticalFOV() { return m_verticalFOV; }
		inline float GetPerspectiveNear() { return m_PerspectiveNear; }
		inline float GetPerspectiveFar() { return m_PerspectiveFar; }
		void SetPerspectiveVerticalFOV(float vFOV) { m_verticalFOV = vFOV; }
		void SetPerspectiveNear(float Near) { m_PerspectiveNear = Near; RecalculateProjection(); }
		void SetPerspectiveFar(float Far) { m_PerspectiveFar = Far; RecalculateProjection(); }

		void RecalculateProjection();//for now it is orthographic projection
	public:
		ProjectionTypes m_projection_type = ProjectionTypes::Orhtographic;
	private:
		float m_verticalFOV = 45.0f;
		float m_PerspectiveNear = 0.01;
		float m_PerspectiveFar = 1000;

		float m_OrthographicSize = 10.f;
		float m_OrthographicFar = 1.f, m_OrthographicNear = -1.f;
		float m_AspectRatio=1.0f;
	};
}