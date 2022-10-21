#include "hzpch.h"
#include "Entity.h"
namespace Hazel {
	Entity::Entity(Scene& scene,entt::entity& entity)
		:entity(entity),scene(scene)
	{
		
	}
}
