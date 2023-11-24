#pragma once
#include "Scene.h"
#include <future>

namespace Hazel {
	class Material;
	class SceneSerializer {
	public:
		SceneSerializer() = default;
		SceneSerializer(const ref<Scene>& scene)
			:m_scene(scene)
		{}
		void Serialize(const std::string& filepath);//serialize to the file path
		void SerializeMaterial(const std::string& filepath, Material& material);
		void SerializeMesh(const std::string& filepath, LoadMesh& mesh);
		void SerializeRuntime(const std::string& filepath);

		void DeSerialize(const std::string& filepath);
		void DeSerializeMaterial(const std::string& filepath, Material& material);
		uint64_t DeSerializeAndGetMaterialID(const std::string& filepath);
		void DeSerializeMesh(const std::string& filepath, LoadMesh& mesh);
		void DeSerializeRuntime(const std::string& filepath);
		//void SerializeEntity(YAML::Emitter& out, Entity& entity);
	private:
		ref<Scene> m_scene;
		std::vector<std::future<void>> m_future;
	};
}