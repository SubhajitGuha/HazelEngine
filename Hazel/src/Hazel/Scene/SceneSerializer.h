#pragma once
#include "Scene.h"
#include <future>

namespace Hazel {
	class SceneSerializer {
	public:
		SceneSerializer(const ref<Scene>& scene)
			:m_scene(scene)
		{}
		void Serialize(const std::string& filepath);//serialize to the file path
		void SerializeRuntime(const std::string& filepath);

		void DeSerialize(const std::string& filepath);
		void DeSerializeRuntime(const std::string& filepath);
		//void SerializeEntity(YAML::Emitter& out, Entity& entity);
	private:
		ref<Scene> m_scene;
		std::vector<std::future<void>> m_future;
	};
}