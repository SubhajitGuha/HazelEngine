#include "hzpch.h"
#include "Entity.h"
#include "Component.h"
#include "SceneSerializer.h"
#include "Hazel/Physics/Physics3D.h"
#include "PointLight.h"
#include "yaml-cpp/yaml.h"

namespace YAML {

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

}

namespace Hazel {
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat4& matrix)
	{
		out << YAML::BeginSeq << matrix[0] << matrix[1] << matrix[2] << matrix[3];
		out << YAML::EndSeq;
		return out;
	}

	static void SerializeEntity(YAML::Emitter& out, Entity& entity)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << "13212514564232";
		
		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;

			auto& tag = entity.GetComponent<TagComponent>();
			out << YAML::Key << "tag" << YAML::Value << tag.tag;
			out << YAML::EndMap;
		}
		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;
			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;
			out << YAML::Key << "Transform" << YAML::Value << tc.m_transform;

			out << YAML::EndMap;
		}
		if (entity.HasComponent<SpriteRenderer>())
		{
			out << YAML::Key << "SpriteRenderer";
			out << YAML::BeginMap;
			auto& sr = entity.GetComponent<SpriteRenderer>();
			out << YAML::Key << "Color" << YAML::Value << sr.Color;
			out << YAML::Key << "Roughness" << YAML::Value << sr.m_Roughness;
			out << YAML::Key << "Metallic" << YAML::Value << sr.m_Metallic;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<StaticMeshComponent>())
		{
			auto& sm = entity.GetComponent<StaticMeshComponent>();
			out << YAML::Key << "StaticMeshComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "MeshPath" << YAML::Value << sm.static_mesh->m_path;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<PhysicsComponent>())
		{
			auto& pc = entity.GetComponent<PhysicsComponent>();
			out << YAML::Key << "PhysicsComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "Mass" << YAML::Value << pc.m_mass;
			out << YAML::Key << "StaticFriction" << YAML::Value << pc.m_StaticFriction;
			out << YAML::Key << "DynamicFriction" << YAML::Value << pc.m_DynamicFriction;
			out << YAML::Key << "Restitution" << YAML::Value << pc.m_Restitution;
			out << YAML::Key << "Radius" << YAML::Value << pc.m_radius;
			out << YAML::Key << "Height" << YAML::Value << pc.m_height;
			out << YAML::Key << "HalfExtent" << YAML::Value << pc.m_halfextent;
			out << YAML::Key << "Transform" << YAML::Value << pc.m_transform;
			out << YAML::Key << "isStatic" << YAML::Value << (int)pc.isStatic;
			out << YAML::Key << "isKinamatic" << YAML::Value << (int)pc.isKinamatic;
			out << YAML::Key << "Shape" << YAML::Value << pc.m_shapes;

			out << YAML::EndMap;
		}
		out << YAML::EndMap;
	}
	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";

		out << YAML::Key << "PointLight";
		out << YAML::BeginSeq;
		for (int i = 0; i < m_scene->m_PointLights.size(); i++) {
			std::string light_name = "PointLight" + std::to_string(i);
			out << YAML::BeginMap;
			out << YAML::Key << light_name.c_str();
			out << YAML::BeginMap;
			out << YAML::Key << "LightPosition" << YAML::Value << m_scene->m_PointLights[i]->GetLightPosition();
			out << YAML::Key << "LightColor" << YAML::Value << m_scene->m_PointLights[i]->GetLightColor();
			out << YAML::EndMap;//Light end map
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;//Light end map

		out << YAML::Key << "Entities" << YAML::BeginSeq;
		m_scene->getRegistry().each([&](auto entity_ID) {
			Entity entity(m_scene.get(), entity_ID);
			SerializeEntity(out, entity);
			});
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream output(filepath);
		output << out.c_str();
	}
	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
	}
	void SceneSerializer::DeSerialize(const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream sstream;
		sstream << stream.rdbuf();

		YAML::Node data = YAML::Load(sstream.str());
		if (!data["Scene"])
		{
			HAZEL_CORE_ERROR("Not a valid scene");
			return;
		}
		std::string SceneName = data["Scene"].as<std::string>();
		HAZEL_CORE_INFO("Deserializing Scene {}", SceneName);

		auto pointLight = data["PointLight"];
		if (pointLight)
		{
			int i = 0;
			for (auto light : pointLight)
			{
				std::string light_name = "PointLight" + std::to_string(i);
				auto light_param = light[light_name.c_str()];
				if (light_param) {
					glm::vec3 lposition = light_param["LightPosition"].as<glm::vec3>();
					glm::vec3 lcolor = light_param["LightColor"].as<glm::vec3>();

					m_scene->AddPointLight(new PointLight(lposition, lcolor));
				}
				i++;
			}
		}

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>(); // TODO

				std::string entity_name;
				auto tagcmp = entity["TagComponent"];
				if (tagcmp)
				{
					entity_name = tagcmp["tag"].as<std::string>();
				}
				Entity* DeserializedEntity = m_scene->CreateEntity(entity_name);

				auto TransformComp = entity["TransformComponent"];
				if (TransformComp)
				{
					glm::vec3 translation = TransformComp["Translation"].as<glm::vec3>();
					glm::vec3 rotation = TransformComp["Rotation"].as<glm::vec3>();
					glm::vec3 scale = TransformComp["Scale"].as<glm::vec3>();
					DeserializedEntity->AddComponent<TransformComponent>(translation,rotation,scale);
				}

				auto StaticMeshComp = entity["StaticMeshComponent"];
				if (StaticMeshComp)
				{
					std::string mesh_path = StaticMeshComp["MeshPath"].as<std::string>();
					LoadMesh* mesh = new LoadMesh(mesh_path);

					if (DeserializedEntity->HasComponent<StaticMeshComponent>())
						DeserializedEntity->ReplaceComponent<StaticMeshComponent>(mesh);
					else
						DeserializedEntity->AddComponent<StaticMeshComponent>(mesh);
				}

				auto SpriteRenderComp = entity["SpriteRenderer"];
				if (SpriteRenderComp)
				{
					glm::vec4 color = SpriteRenderComp["Color"].as<glm::vec4>();
					float roughness = SpriteRenderComp["Roughness"].as<float>();
					float metallic = SpriteRenderComp["Metallic"].as<float>();

					DeserializedEntity->AddComponent<SpriteRenderer>(color,roughness,metallic);
				}

				auto PhysicsComp = entity["PhysicsComponent"];
				if (PhysicsComp)
				{
					DeserializedEntity->AddComponent<PhysicsComponent>();
					auto& physics_component = DeserializedEntity->GetComponent<PhysicsComponent>();

					float mass = PhysicsComp["Mass"].as<float>();
					float staticFriction = PhysicsComp["StaticFriction"].as<float>();
					float dynamicFriction = PhysicsComp["DynamicFriction"].as<float>();
					float restitution = PhysicsComp["Restitution"].as<float>();
					float radius = PhysicsComp["Radius"].as<float>();
					float height = PhysicsComp["Height"].as<float>();
					glm::vec3 halfExtent = PhysicsComp["HalfExtent"].as<glm::vec3>();
					//glm::mat4 transform = PhysicsComp["Transform"].as<glm::mat4>();
					bool isStatic = PhysicsComp["isStatic"].as<int>();
					bool isKinamatic = PhysicsComp["isKinamatic"].as<int>();
					int Shape = PhysicsComp["Shape"].as<int>();

					physics_component.m_mass = mass;
					physics_component.m_StaticFriction = staticFriction;
					physics_component.m_DynamicFriction = dynamicFriction;
					physics_component.m_Restitution = restitution;
					physics_component.m_radius = radius;
					physics_component.m_height = height;
					physics_component.m_halfextent = halfExtent;
					physics_component.m_transform = DeserializedEntity->GetComponent<TransformComponent>().GetTransform();
					physics_component.isStatic = isStatic;
					physics_component.isKinamatic = isKinamatic;
					physics_component.m_shapes = (ShapeTypes)Shape;

					if (Shape == ShapeTypes::BOX_COLLIDER)
						Physics3D::AddBoxCollider(physics_component);
					if (Shape == ShapeTypes::SPHERE_COLLIDER)
						Physics3D::AddSphereCollider(physics_component);
					if (Shape == ShapeTypes::PLANE_COLLIDER)
						Physics3D::AddPlaneCollider(physics_component);
					if (Shape == ShapeTypes::CAPSULE_COLLIDER)
						Physics3D::AddCapsuleCollider(physics_component);
					if (Shape == ShapeTypes::MESH_COLLIDER)
					{
						if (DeserializedEntity->HasComponent<StaticMeshComponent>())
						{
							auto& mesh = DeserializedEntity->GetComponent<StaticMeshComponent>();
							Physics3D::AddMeshCollider(mesh.static_mesh->Vertices, mesh.static_mesh->Vertex_Indices, DeserializedEntity->GetComponent<TransformComponent>().Scale, physics_component);
						}
					}
				}
			}
		}
	}
	void SceneSerializer::DeSerializeRuntime(const std::string& filepath)
	{
	}
}