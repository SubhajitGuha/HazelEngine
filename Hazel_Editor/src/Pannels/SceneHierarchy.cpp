#include "SceneHierarchy.h"
#include "glm/gtc/type_ptr.hpp"
#include "imgui_internal.h"
#include "imgui.h"
namespace Hazel {
	std::string texture_path = "Assets/Textures/Test.png";
	SceneHierarchyPannel::SceneHierarchyPannel() = default;
	SceneHierarchyPannel::~SceneHierarchyPannel()
	{}

	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	void SceneHierarchyPannel::Context(const ref<Scene>& context)
	{
		m_Context = context;
	}
	void SceneHierarchyPannel::OnImGuiRender()
	{
		//static int 
		bool isEntityDestroyed = false;
		ref<Entity> m_Entity;
		ImGuiTreeNodeFlags flags = 0;
		ImGui::Begin("Scene Hierarchy Pannel");
		
			if (ImGui::BeginPopupContextWindow("Actions",1,false))
			{
				if (ImGui::Button("Create Entity", { 120,30 }))
				{
					m_Context->CreateEntity();
				}
				ImGui::EndPopup();
			}
			m_Context->getRegistry().each([&](auto entity) {

				m_Entity = std::make_shared<Entity>(m_Context.get(), entity);

				std::string s = m_Entity->GetComponent<TagComponent>();

				if (m_selected_entity)
					flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow | ((*m_selected_entity == *m_Entity) ? ImGuiTreeNodeFlags_Selected : 0);

				bool opened = ImGui::TreeNodeEx((void*)(uint32_t)*m_Entity, flags, s.c_str());

				if (ImGui::IsItemClicked())
					m_selected_entity = m_Entity;//gets casted to uint32_t
				if (opened)
				{
					ImGui::TreePop();
				}
				});

		if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
			m_selected_entity = nullptr;

		if (m_selected_entity && ImGui::BeginPopupContextItem("Action"))//popup only appears when there is a selected entity
		{
			if (ImGui::Button("Delete Entity", { 100,30 })) {
				isEntityDestroyed = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::End();

		//properties pannel
		ImGui::Begin("Properties");

		if (m_selected_entity && ImGui::BeginPopupContextWindow("Actions",1,false))//click on any blank area to add component
		{
			if (ImGui::Button("Create Transform Component", { 220,30 }))
			{
				m_selected_entity->AddComponent<TransformComponent>();//sets to default values
			}
			
			if (ImGui::Button("Create Sprite Renderer Component", { 220,30 }))
			{
				m_selected_entity->AddComponent<SpriteRenderer>();
			}
			if (ImGui::Button("Create Camera Component", { 220,30 }))
			{
				m_selected_entity->AddComponent<CameraComponent>();
			}
			ImGui::EndPopup();
		}
		if (m_selected_entity.get() && m_selected_entity->HasComponent<TagComponent>())
		{
			DrawTagUI();
		}

		if (m_selected_entity.get() && m_selected_entity->HasComponent<TransformComponent>())
		{
			DrawTransformUI();
		}

		if (m_selected_entity.get() && m_selected_entity->HasComponent<CameraComponent>())
		{
			DrawCameraUI();
		}

		if (m_selected_entity.get() && m_selected_entity->HasComponent<SpriteRenderer>())
		{
			DrawSpriteRendererUI();
		
		}
		ImGui::End();

		if (isEntityDestroyed)//delete entity
		{
			m_Context->DestroyEntity(*m_selected_entity);
			m_selected_entity = nullptr;
		}
	}
	void SceneHierarchyPannel::DrawHierarchyNode(const Entity* ent)
	{
	}
	void SceneHierarchyPannel::DrawProperties()
	{
	}

	void SceneHierarchyPannel::DrawTransformUI()
	{
		if (!m_selected_entity)
			return;
		auto& transform = m_selected_entity->GetComponent<TransformComponent>();
		if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen, "Transform Component")) //typeid() operator is a part of runtime type identification in cpp it determines the type of object at runtime
		{
			DrawVec3Control("Translation", transform.Translation);
			DrawVec3Control("Rotation", transform.Rotation);
			DrawVec3Control("Scale", transform.Scale, 1.0);

			ImGui::TreePop();
		}
	}

	void SceneHierarchyPannel::DrawCameraUI()
	{
		if (!m_selected_entity)
			return;
		bool open = ImGui::TreeNodeEx(" Camera Component", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen);
		bool delete_component = false;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 2,2 });
		ImGui::SameLine(ImGui::GetWindowWidth() - 40.f);
		if (ImGui::Button("+", { 20,20 }))
		{
			ImGui::OpenPopup("ComponentSettings1");
		}
		ImGui::PopStyleVar();
		if (ImGui::BeginPopup("ComponentSettings1"))
		{
			if (ImGui::MenuItem("Remove Component"))
				delete_component = true;
			ImGui::EndPopup();
		}
		auto& camera = m_selected_entity->GetComponent<CameraComponent>().camera;
		const char* Items[] = { "Orthographic","Perspective" };
		const char* Selected_Item = Items[(int)camera.m_projection_type];
		if (open)
		{
			bool isPrimary = camera.bIsMainCamera;
			ImGui::Checkbox("Primary Camera", &isPrimary);
			camera.bIsMainCamera = isPrimary;

			if (isPrimary)//if the primary camera of one entity is true then set all entities primary camera to false(as you should have only one primary camera)
			{
				m_Context->getRegistry().view<CameraComponent>().each([&](entt::entity entity, CameraComponent& camerac)//get all entities that have camera component
					{
						Entity tmp_entity = Entity(m_Context.get(), entity);
						if (tmp_entity != *m_selected_entity)
						{
							camerac.camera.bIsMainCamera = false;
						}
					});
			}


			if (ImGui::BeginCombo("Projection Types", Selected_Item))
			{
				for (int i = 0; i < 2; i++)
				{
					bool bIsSelected = (Selected_Item == Items[i]);
					if (ImGui::Selectable(Items[i], bIsSelected))
					{
						Selected_Item = Items[i];
						camera.SetProjectionType((ProjectionTypes)i);
					}
					if (bIsSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			if (camera.m_projection_type == ProjectionTypes::Orhtographic)
			{
				float OrthoSize = camera.GetOrthographicSize();
				if (ImGui::DragFloat("Orthographic size", &OrthoSize, 0.2, 0, 1000))
					camera.SetOrthographicSize(OrthoSize);

				float OrthoNear = camera.GetOrthographicNear();
				if (ImGui::DragFloat("Orthographic Near", &OrthoNear, 0.2, -100, 100))
					camera.SetOrthographicNear(OrthoNear);

				float OrthoFar = camera.GetOrthographicFar();
				if (ImGui::DragFloat("Orthographic Far", &OrthoFar, 0.2, 0, 1000))
					camera.SetOrthographicFar(OrthoFar);

			}
			else
			{
				float PersFOV = camera.GetPerspectiveVerticalFOV();
				if (ImGui::DragFloat("Perspective size", &PersFOV, 0.2))
					camera.SetPerspectiveVerticalFOV(PersFOV);

				float PersNear = camera.GetPerspectiveNear();
				if (ImGui::DragFloat("Perspective Near", &PersNear, 0.2))
					camera.SetPerspectiveNear(PersNear);

				float PersFar = camera.GetPerspectiveFar();
				if (ImGui::DragFloat("Perspective Far", &PersFar, 0.2))
					camera.SetPerspectiveFar(PersFar);
			}
			ImGui::TreePop();
		}
		if (delete_component)
			m_selected_entity->RemoveComponent<CameraComponent>();
	}
	void SceneHierarchyPannel::DrawTagUI()
	{
		if (!m_selected_entity)
			return;
		if (ImGui::TreeNodeEx("TAG Component", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
		{
			char buffer[250] = { 0 };
			TagComponent& tag = m_selected_entity->GetComponent<TagComponent>();
			std::string s = tag;
			strcpy_s(buffer, sizeof(buffer), s.c_str());
			if (ImGui::InputText("TAG", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
				
			}
			ImGui::TreePop();
		}
	}
	void SceneHierarchyPannel::DrawSpriteRendererUI()
	{
		if (!m_selected_entity)
			return;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 2,2 });
		bool open = ImGui::TreeNodeEx("Sprite Renderer", ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow);
		bool delete_component = false;
		ImGui::SameLine(ImGui::GetWindowWidth()-40.f);
		if (ImGui::Button("+", {20,20}))
		{
			ImGui::OpenPopup("ComponentSettings");
		}
		ImGui::PopStyleVar();
		if (ImGui::BeginPopup("ComponentSettings"))
		{
			if (ImGui::MenuItem("Remove Component"))
				delete_component = true;
			ImGui::EndPopup();
		}
		if(open)
		{
			auto& Sprite_Renderer = m_selected_entity->GetComponent<SpriteRenderer>();
			ImGui::ColorEdit4("##Sprite", (float*)(&Sprite_Renderer.Color));

			char buf[200];
			strcpy_s(buf,sizeof(buf), texture_path.c_str());
			if (ImGui::InputText("Texture Path", buf, 200))
			{
				texture_path = std::string(buf);
			}

			if (ImGui::Button("APPLY", {100,30}))
			{
				Sprite_Renderer.texture = Texture2D::Create(buf);
			}
			ImGui::TreePop();
		}
		if (delete_component)
			m_selected_entity->RemoveComponent<SpriteRenderer>();
	}
}