#include "hzpch.h"
#include "Entity.h"
namespace Hazel {
	Entity::Entity(Scene* scene,const entt::entity& entity)
		:entity(entity),scene(scene)
	{
	}
}
