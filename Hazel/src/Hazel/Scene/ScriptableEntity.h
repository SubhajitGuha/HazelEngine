#pragma once
#include "Entity.h"
namespace Hazel 
{
	//Script class Which must be inherited to write custom scripts
	class ScriptableEntity
	{
	public:
		Entity* m_Entity = nullptr;
		friend class Scene;
	public:
		ScriptableEntity() = default;
		
		virtual ~ScriptableEntity() {
			//delete m_Entity;
		}

		//overridable methods
		virtual void OnUpdate(TimeStep ts) {}
		virtual void OnCreate() {}
		virtual void OnDestroy() {}
	};
}