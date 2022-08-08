#pragma once
#include"hzpch.h"
#include "Core.h"
#include"Events/Event.h"
namespace Hazel {
	class HAZEL_API Layer
	{
		public:
			Layer(const std::string s = "Layer");
			virtual ~Layer();
			virtual void OnUpdate() {}
			virtual void OnAttach(){}
			virtual void OnDetach(){}
			virtual void OnEvent(Event& e){}
			inline std::string GetName()const { return m_DebugName; }
	private:
		std::string m_DebugName;

	};
}

