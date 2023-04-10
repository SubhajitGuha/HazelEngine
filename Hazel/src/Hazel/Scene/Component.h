#pragma once
#include "Hazel.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Hazel/Renderer/Camareas/SceneCamera.h"
#include "Hazel/LoadMesh.h"
#include "ScriptableEntity.h"

namespace Hazel {
	struct TagComponent {
		std::string tag;
		TagComponent() {tag = "";}
		TagComponent(const std::string& name)
			:tag(name){}
		operator std::string() { return tag; }
	};
	struct TransformComponent {
		glm::vec3 Translation = { 0,0,0 };
		glm::vec3 Rotation = { 0,0,0 };//in degrees
		glm::vec3 Scale = { 1,1,1 };
		TransformComponent() = default;
		TransformComponent(const glm::vec3& translation,const glm::vec3& rotatation=glm::vec3(0),const glm::vec3& scale=glm::vec3(1))
			:Translation(translation),Rotation(rotatation),Scale(scale){}
		glm::mat4 GetTransform() {
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0), glm::radians(Rotation.x), { 1,0,0 }) *
				glm::rotate(glm::mat4(1.0), glm::radians(Rotation.y), { 0,1,0 }) *
				glm::rotate(glm::mat4(1.0), glm::radians(Rotation.z), { 0,0,1 });
			return glm::translate(glm::mat4(1.0), Translation) * rotation * glm::scale(glm::mat4(1), Scale);
		}
	};

	struct CameraComponent {
		SceneCamera camera;
		CameraComponent() 
			:camera() 
		{}
		CameraComponent(float width,float height)
			:camera(width,height)
		{}
		operator Camera& () { return camera; }
	};

	struct ScriptComponent {
		ScriptableEntity* m_Script = nullptr;
		std::function<void()> CreateInstance;
		std::function<void()> DeleteInstance;

		//call bind function at the time of attaching the component and pass the inherited custom class of ScriptableEntity as template
		template<typename t>
		void Bind()
		{
			CreateInstance = [&]() {m_Script = new t(); };
			DeleteInstance = [&]() {delete m_Script; };
		}
	};

	class Texture2D;
	struct SpriteRenderer
	{
		glm::vec4 Color = pow(glm::vec4(1.0),glm::vec4(GAMMA));
		float m_Roughness = 1.0f;
		float m_Metallic = 0.0f;
		ref<Texture2D> texture = nullptr;
		SpriteRenderer() = default;
		SpriteRenderer(const glm::vec4& color, float roughness = 1.0f, float metallic = 0.0f, const ref<Texture2D> tex = nullptr)
			:Color(pow(color,glm::vec4(GAMMA))),texture(tex),m_Metallic(metallic),m_Roughness(roughness)//Gamma correction in color
		{}
	};

	struct StaticMeshComponent
	{
		LoadMesh* static_mesh = nullptr;
		bool isFoliage = false;
		StaticMeshComponent() = default;
		StaticMeshComponent(LoadMesh* staticmesh)
			:static_mesh(staticmesh)
		{}
		operator LoadMesh* () { return static_mesh; }
	};

}