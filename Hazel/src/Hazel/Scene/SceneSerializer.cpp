#include "hzpch.h"
#include "Entity.h"
#include "Component.h"
#include "SceneSerializer.h"
#include "Hazel/Physics/Physics3D.h"
#include "PointLight.h"
#include "yaml-cpp/yaml.h"
#include "Hazel/LoadMesh.h"
#include "Hazel/Renderer/Material.h"

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

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;
			auto& cc = entity.GetComponent<CameraComponent>();
			auto& camera = cc.camera;
			out << YAML::Key << "Camera Distance" << YAML::Value << cc.camera_dist;
			out << YAML::Key << "Follow Player" << YAML::Value << (int)cc.bFollowPlayer;
			out << YAML::Key << "FOV" << YAML::Value << camera.GetVerticalFOV();
			out << YAML::Key << "Aspect Ratio" << YAML::Value << camera.GetAspectRatio();
			out << YAML::Key << "Near" << YAML::Value << camera.GetPerspectiveNear();
			out << YAML::Key << "Far" << YAML::Value << camera.GetPerspectiveFar();
			out << YAML::Key << "View Direction" << YAML::Value << camera.GetViewDirection();
			out << YAML::Key << "Is Main Camera" << YAML::Value << (int)camera.bIsMainCamera;

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
		if (entity.HasComponent<ScriptComponent>())
		{
			auto& sm = entity.GetComponent<ScriptComponent>();
			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "id" << YAML::Value << typeid(*sm.m_Script).hash_code();
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
			out << YAML::Key << "isKinematic" << YAML::Value << (int)pc.isKinematic;
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
	void SceneSerializer::SerializeMaterial(const std::string& filepath, Material& material)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Material ID" << YAML::Value << material.materialID;
		out << YAML::Key << "Color" << YAML::Value << material.GetColor();
		out << YAML::Key << "Roughness" << YAML::Value << material.GetRoughness();
		out << YAML::Key << "Metallic" << YAML::Value << material.GetMetalness();
		out << YAML::Key << "Normal Strength" << YAML::Value << material.GetNormalStrength();
		out << YAML::Key << "Emission" << YAML::Value << material.GetEmission();
		out << YAML::Key << "Albedo Map" << YAML::Value << material.GetAlbedoPath();
		out << YAML::Key << "Normal Map" << YAML::Value << material.GetNormalPath();
		out << YAML::Key << "Roughness Map" << YAML::Value << material.GetRoughnessPath();
		out << YAML::Key << "Shader Path" << YAML::Value << "TODO";
		out << YAML::EndMap;

		std::ofstream output(filepath);
		output << out.c_str();
	}
	void SceneSerializer::SerializeMesh(const std::string& filepath, LoadMesh& mesh)
	{
		/*
			UUID : 125663
			0:
				(uin64_t)materialID
				(vec3)vertices
				(vec3)normal
				(vec3)tangent
				(vec2)texCoord
------------------------------------------------------------------------------
				(vec3)vertices
				(vec3)normal
				(vec3)tangent
				(vec2)texCoord
		*/

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "UUID" << YAML::Value << YAML::Binary(reinterpret_cast<const unsigned char*>(&mesh.uuid),sizeof(uint64_t));
		for (int i = 0; i < mesh.m_subMeshes.size(); i++)
		{
			auto sub_mesh = mesh.m_subMeshes[i];
			out << YAML::Key << std::to_string(i) << YAML::Value;
			out << YAML::BeginSeq;
			out << YAML::Binary(reinterpret_cast<const unsigned char*>(&sub_mesh.m_Material->materialID), sizeof(uint64_t));
			for (int k = 0; k < sub_mesh.numVertices; k++)
			{
				out << YAML::Binary(reinterpret_cast<const unsigned char*>(&sub_mesh.Vertices[k].x), sizeof(float) * 3);
				out << YAML::Binary(reinterpret_cast<const unsigned char*>(&sub_mesh.Normal[k].x), sizeof(float) * 3);
				out << YAML::Binary(reinterpret_cast<const unsigned char*>(&sub_mesh.Tangent[k].x), sizeof(float) * 3);
				out << YAML::Binary(reinterpret_cast<const unsigned char*>(&sub_mesh.TexCoord[k].x), sizeof(float) * 2);
			}
			out << YAML::EndSeq;
		}
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
					DeserializedEntity->AddComponent<TransformComponent>(translation, rotation, scale);
				}

				auto StaticMeshComp = entity["StaticMeshComponent"];
				if (StaticMeshComp)
				{
					std::string mesh_path = StaticMeshComp["MeshPath"].as<std::string>();
					LoadMesh* mesh = new LoadMesh(mesh_path);
					//m_future.push_back(std::async(std::launch::async, LoadMeshes, mesh_path, mesh));

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

					DeserializedEntity->AddComponent<SpriteRenderer>(color, roughness, metallic);
				}
				auto ScriptComp = entity["ScriptComponent"];
				if (ScriptComp)
				{
					if (ScriptComp["id"]) {
						auto script = m_scene->m_scriptsMap[ScriptComp["id"].as<size_t>()];
						DeserializedEntity->AddComponent<ScriptComponent>().Bind(*script);
					}
				}

				auto CameraComp = entity["CameraComponent"];
				if (CameraComp)
				{
					auto& cc = DeserializedEntity->AddComponent<CameraComponent>();
					if (CameraComp["Camera Distance"])
						cc.camera_dist = CameraComp["Camera Distance"].as<glm::vec3>();
					if (CameraComp["Follow Player"])
						cc.bFollowPlayer = (bool)CameraComp["Follow Player"].as<int>();
					if (CameraComp["FOV"])
						cc.camera.SetVerticalFOV(CameraComp["FOV"].as<float>());
					if (CameraComp["Aspect Ratio"])
						cc.camera.SetViewportSize(CameraComp["Aspect Ratio"].as<float>());
					if (CameraComp["Near"])
						cc.camera.SetOrthographicNear(CameraComp["Near"].as<float>());
					if (CameraComp["Far"])
						cc.camera.SetOrthographicFar(CameraComp["Far"].as<float>());
					if (CameraComp["View Direction"])
						cc.camera.SetViewDirection(CameraComp["View Direction"].as<glm::vec3>());
					if (CameraComp["Is Main Camera"])
						cc.camera.bIsMainCamera = (bool)CameraComp["Is Main Camera"].as<int>();
				}

				auto PhysicsComp = entity["PhysicsComponent"];
				if (PhysicsComp)
				{
					auto& physics_component = DeserializedEntity->AddComponent<PhysicsComponent>();

					if (PhysicsComp["Mass"])
						physics_component.m_mass = PhysicsComp["Mass"].as<float>();

					if (PhysicsComp["StaticFriction"])
						physics_component.m_StaticFriction = PhysicsComp["StaticFriction"].as<float>();

					if (PhysicsComp["DynamicFriction"])
						physics_component.m_DynamicFriction = PhysicsComp["DynamicFriction"].as<float>();

					if (PhysicsComp["Restitution"])
						physics_component.m_Restitution = PhysicsComp["Restitution"].as<float>();

					if (PhysicsComp["Radius"])
						physics_component.m_radius = PhysicsComp["Radius"].as<float>();

					if (PhysicsComp["Height"])
						physics_component.m_height = PhysicsComp["Height"].as<float>();

					if (PhysicsComp["HalfExtent"])
						physics_component.m_halfextent = PhysicsComp["HalfExtent"].as<glm::vec3>();

					if (PhysicsComp["isStatic"])
						physics_component.isStatic = PhysicsComp["isStatic"].as<int>();

					if (PhysicsComp["isKinematic"])
						physics_component.isKinematic = PhysicsComp["isKinematic"].as<int>();

					if (PhysicsComp["Shape"])
						physics_component.m_shapes = (ShapeTypes)PhysicsComp["Shape"].as<int>();

					physics_component.m_transform = DeserializedEntity->GetComponent<TransformComponent>().GetTransform();

					if (physics_component.m_shapes == ShapeTypes::BOX_COLLIDER)
						Physics3D::AddBoxCollider(physics_component);
					if (physics_component.m_shapes == ShapeTypes::SPHERE_COLLIDER)
						Physics3D::AddSphereCollider(physics_component);
					if (physics_component.m_shapes == ShapeTypes::PLANE_COLLIDER)
						Physics3D::AddPlaneCollider(physics_component);
					if (physics_component.m_shapes == ShapeTypes::CAPSULE_COLLIDER)
						Physics3D::AddCapsuleCollider(physics_component);
					//if (physics_component.m_shapes == ShapeTypes::MESH_COLLIDER)
					//{
					//	if (DeserializedEntity->HasComponent<StaticMeshComponent>())
					//	{
					//		auto& mesh = DeserializedEntity->GetComponent<StaticMeshComponent>();
					//		Physics3D::AddMeshCollider(mesh.static_mesh->Vertices, mesh.static_mesh->Vertex_Indices, DeserializedEntity->GetComponent<TransformComponent>().Scale, physics_component);
					//	}
					//}
				}
			}
		}
	}
	void SceneSerializer::DeSerializeMaterial(const std::string& filepath, Material& material)
	{
		std::ifstream inputfile(filepath);
		YAML::Node data = YAML::Load(inputfile);

		HZ_ASSERT(data["Material ID"], "Invalid key");
		HZ_ASSERT(data["Color"], "Invalid key");
		HZ_ASSERT(data["Roughness"], "Invalid key");
		HZ_ASSERT(data["Metallic"], "Invalid key");
		HZ_ASSERT(data["Emission"], "Invalid key");
		HZ_ASSERT(data["Normal Strength"], "Invalid key");
		HZ_ASSERT(data["Albedo Map"], "Invalid key");
		HZ_ASSERT(data["Normal Map"], "Invalid key");
		HZ_ASSERT(data["Roughness Map"], "Invalid key");

		material.materialID = data["Material ID"].as<uint64_t>();
		glm::vec3 color = data["Color"].as<glm::vec3>();
		float roughness = data["Roughness"].as<float>();
		float metallic = data["Metallic"].as<float>();
		float normal_strength = data["Normal Strength"].as<float>();
		float emission = data["Emission"].as<float>();
		material.SetEmission(emission);
		material.SetMaterialAttributes(color, roughness, metallic, normal_strength);
		std::string albedo_path = data["Albedo Map"].as<std::string>();
		std::string normal_path = data["Normal Map"].as<std::string>();
		std::string roughness_path = data["Roughness Map"].as<std::string>();
		material.SetTexturePaths(albedo_path, normal_path, roughness_path);
		//TODO Shader

	}
	void SceneSerializer::DeSerializeMesh(const std::string& filepath, LoadMesh& mesh)
	{
		std::ifstream inputFile(filepath);
		YAML::Node data = YAML::Load(inputFile);

		{	//mesh UUID decript from binary
			YAML::Binary bin_data = data["UUID"].as<YAML::Binary>();
			const unsigned char* binary = bin_data.data();
			std::memcpy(&mesh.uuid, binary, bin_data.size()); //get uuid
		}
		int k = 0;
		while (true)
		{
			if (data[std::to_string(k)])
			{
				auto value = data[std::to_string(k)];
				std::vector<glm::vec3> vertex_pos, vertex_normal, vertex_tangent, vertex_bitangent;
				std::vector<glm::vec2> texture_coord;
				uint64_t materialID;

				{	//materialID decript from binary
					YAML::Binary bin_data = value[0].as<YAML::Binary>();
					const unsigned char* binary = bin_data.data();
					std::memcpy(&materialID, binary, bin_data.size()); //get uuid
				}
				for (int i = 1; i < value.size(); i+=4)
				{
					{	//position decript from binary
						YAML::Binary bin_data = value[i].as<YAML::Binary>();
						const unsigned char* binary = bin_data.data();
						float vertex_data[3];
						std::memcpy(vertex_data, binary, bin_data.size()); //get vertex position
						vertex_pos.push_back({ vertex_data[0],vertex_data[1],vertex_data[2] });
					}
					{	//normal decript from binary
						YAML::Binary bin_data = value[i+1].as<YAML::Binary>();
						const unsigned char* binary = bin_data.data();
						float vertex_data[3];
						std::memcpy(vertex_data, binary, bin_data.size()); //get vertex normal
						vertex_normal.push_back({ vertex_data[0],vertex_data[1],vertex_data[2] });
					}
					{	//tangent and bi_tangent decript from binary
						YAML::Binary bin_data = value[i+2].as<YAML::Binary>();
						const unsigned char* binary = bin_data.data();
						float vertex_data[3];
						std::memcpy(vertex_data, binary, bin_data.size()); //get vertex tangent
						vertex_tangent.push_back({ vertex_data[0],vertex_data[1],vertex_data[2] });

						glm::vec3 bitangent = glm::cross(vertex_tangent.back(), vertex_normal.back());
						vertex_bitangent.push_back(bitangent);
					}
					{	//tex coord decript from binary
						YAML::Binary bin_data = value[i+3].as<YAML::Binary>();
						const unsigned char* binary = bin_data.data();
						float vertex_data[2];
						std::memcpy(vertex_data, binary, bin_data.size()); //get uuid
						texture_coord.push_back({ vertex_data[0],vertex_data[1]});
					}
				}
				LoadMesh::SubMesh sub_mesh;
				sub_mesh.Vertices = vertex_pos;
				sub_mesh.Normal = vertex_normal;
				sub_mesh.Tangent = vertex_tangent;
				sub_mesh.BiTangent = vertex_bitangent;
				sub_mesh.TexCoord = texture_coord;
				sub_mesh.numVertices = vertex_pos.size();
				sub_mesh.m_Material = Material::allMaterials[materialID];

				mesh.m_subMeshes.push_back(sub_mesh);
			}
			else
				break;
			k++;
		}
	}
	void SceneSerializer::DeSerializeRuntime(const std::string& filepath)
	{
	}
}