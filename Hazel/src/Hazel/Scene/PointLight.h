#pragma once
#include "Hazel.h"

namespace Hazel {
	class PointLight
	{
	public:
		PointLight() = default;
		PointLight(glm::vec3 pos,glm::vec3 color,std::string tag = "PointLight")
			:m_LightPosition(pos),m_LightColor(color),m_Light_Tag(tag)
		{}
		inline void SetLightPosition(const glm::vec3& pos) { m_LightPosition = pos; }
		inline void SetLightColor(const glm::vec3& color) { m_LightColor = color; }
		inline glm::vec3 GetLightPosition() { return m_LightPosition; }
		inline glm::vec3 GetLightColor() { return m_LightColor; }
		inline void SetLightTag(std::string& tag) { m_Light_Tag = tag; }
		inline std::string& GetLightTag() { return m_Light_Tag;}
	private:
		std::string m_Light_Tag;
		glm::vec3 m_LightPosition;
		glm::vec3 m_LightColor;
		float m_attenuation = 1.0f;
	};
}