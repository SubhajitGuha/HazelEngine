#pragma once
#include "Entity.h"
#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/KeyEvent.h"
#include "Hazel/Events/MouseEvent.h"

namespace Hazel 
{
	//Script class Which must be inherited to write custom scripts
	class ScriptableEntity
	{
	public:
		Entity* m_Entity = nullptr;
		friend class Scene;
		std::pair<size_t, ScriptableEntity*> m_scriptPair;

	public:
		ScriptableEntity() = default;
		
		virtual ~ScriptableEntity() {
			//delete m_Entity;
		}

		//overridable methods
		virtual void OnUpdate(TimeStep ts){}
		virtual void OnEvent(Event& e) {}
		virtual void OnCreate(){}
		virtual void OnDestroy(){}

	};
	//std::unordered_map<size_t, ScriptableEntity*> ScriptableEntity::m_scriptMap = {};
}