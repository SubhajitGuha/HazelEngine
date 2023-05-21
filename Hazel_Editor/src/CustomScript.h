#pragma once
#include "Hazel/Scene/ScriptableEntity.h"
#include "Hazel.h"
#include "Hazel//Physics/Physics3D.h"
namespace Hazel {
	class CustomScript :public ScriptableEntity {
	public:
		glm::vec3 position = { 0,0,0 }, rotate = { 0,0,0 };
		glm::vec2 OldMousePos = { 0,0 };

		Physics3D physics_obj;
		Entity* s_entity;
		// float rotate = 0.0;
		float ObjSpeed = 10;
		float scale = 1;
		float size = 1.0f;
		virtual void OnUpdate(TimeStep ts) override
		{
			if (!m_Entity)
				return;

			if (Input::IsKeyPressed(HZ_KEY_W))
				position.z += ObjSpeed * ts;
			if (Input::IsKeyPressed(HZ_KEY_S))
				position.z -= ObjSpeed * ts;
			if (Input::IsKeyPressed(HZ_KEY_Q))
				rotate.y -= 1;
			if (Input::IsKeyPressed(HZ_KEY_E))
				rotate.y += 1;
			if (Input::IsKeyPressed(HZ_KEY_UP))
				position.y += ObjSpeed * ts;
			if (Input::IsKeyPressed(HZ_KEY_DOWN))
				position.y -= ObjSpeed * ts;
			if (Input::IsKeyPressed(HZ_KEY_A))
				position.x += ObjSpeed * ts;
			if (Input::IsKeyPressed(HZ_KEY_D))
				position.x -= ObjSpeed * ts;
			
			if (Input::IsKeyPressed(HZ_KEY_PAGE_UP))
				scale += 0.01;
			if (Input::IsKeyPressed(HZ_KEY_PAGE_DOWN))
				scale -= 0.01;

			if (m_Entity->HasComponent<CameraComponent>()) {
				auto camera = m_Entity->GetComponent<CameraComponent>().camera;
				if (Input::IsButtonPressed(HZ_MOUSE_BUTTON_1))
				{
					
					physics_obj.Raycast(m_Entity->GetComponent<TransformComponent>().Translation, camera.GetViewDirection(), 10000);
				}
				if (physics_obj.Hit.isHit) {
					Renderer2D::LineBeginScene(camera);
					Renderer2D::DrawLine(m_Entity->GetComponent<TransformComponent>().Translation, physics_obj.Hit.Position, { 1,0.6,1,1 });
					Renderer2D::LineEndScene();
					//HAZEL_CORE_INFO("Ray cast result: {}",physics_obj.Hit.isHit);
				}
			}

			//if (Input::IsButtonPressed(HZ_MOUSE_BUTTON_1))
			//{
			//	s_entity = m_Entity->GetScene()->CreateEntity("Spwanned");
			//	auto mouse_pos = Input::GetCursorPosition();
			//	auto window_size = RenderCommand::GetViewportSize();
			//	glm::vec4 pos = { mouse_pos.first/(window_size.x*0.5) - 1.0, mouse_pos.second/(window_size.y*0.5)-1.0 ,-10,1.0 };
			//	EditorCamera cam;
			//	auto wp = m_Entity->GetComponent<TransformComponent>();
			//	s_entity->AddComponent<TransformComponent>().m_transform = wp.m_transform;
			//	auto& physics_comp = s_entity->AddComponent<PhysicsComponent>();
			//	physics_comp.m_mass = 1000.0f;
			//	physics_comp.m_halfextent = glm::vec3(0.5);
			//	physics_comp.m_transform = s_entity->GetComponent<TransformComponent>().GetTransform();
			//	physics_comp.m_ForceDirection = { 80,-21,0.1 };
			//	Physics3D::AddBoxCollider(physics_comp);
			//}
			//if (!m_Entity->HasComponent<SpriteRenderer>())
				//m_Entity->AddComponent<SpriteRenderer>(glm::vec4(0, 0.3, 1, 1));
			//auto transform = glm::translate(glm::mat4(1.f), position) * glm::rotate(glm::mat4(1.0f),glm::radians(rotate),glm::vec3(0,0,1)) * glm::scale(glm::mat4(1.f), glm::vec3(scale));
			m_Entity->ReplaceComponent<TransformComponent>(position, rotate);//controlling transform
			//m_Entity->GetComponent<CameraComponent>().camera.SetOrthographic(size);//controlling camera
		}
		virtual void OnCreate() override {}
		virtual void OnDestroy() override {}
	};
}