#include "SceneHierarchy.h"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
namespace Hazel {
	SceneHierarchyPannel::SceneHierarchyPannel() = default;
	SceneHierarchyPannel::~SceneHierarchyPannel()
	{}
	void SceneHierarchyPannel::Context(const ref<Scene>& context)
	{
		m_Context = context;
	}
	void SceneHierarchyPannel::OnImGuiRender()
	{
		//static int 
		ref<Entity> m_Entity;
		ImGuiTreeNodeFlags flags = 0;
		ImGui::Begin("Scene Hierarchy Pannel");
		{
			m_Context->getRegistry().each([&](auto entity) {

				m_Entity = std::make_shared<Entity>(m_Context.get(), entity);

				std::string s = m_Entity->GetComponent<TagComponent>();

				if (m_selected_entityID)
					flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow | ((*m_selected_entityID == *m_Entity) ? ImGuiTreeNodeFlags_Selected : 0);

				bool opened = ImGui::TreeNodeEx((void*)(uint32_t)*m_Entity, flags, s.c_str());

				if (ImGui::IsItemClicked())
					m_selected_entityID = m_Entity;//gets casted to uint32_t
				if (opened)
				{
					ImGui::TreePop();
				}
				});
		}
		if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
			m_selected_entityID = nullptr;
		ImGui::End();

		ImGui::Begin("Properties");
		if (m_selected_entityID.get() && m_selected_entityID->HasComponent<TagComponent>())
		{
			char buffer[250] = {0};
			TagComponent& tag = m_selected_entityID->GetComponent<TagComponent>();
			std::string s = tag;
			strcpy_s(buffer, sizeof(buffer), s.c_str());
			if (ImGui::InputText("TAG", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}
		if (m_selected_entityID.get() && m_selected_entityID->HasComponent<TransformComponent>())
		{
			if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), 0, "Position")) //typeid() operator is a part of runtime type identification in cpp it determines the type of object at runtime
			{
				glm::mat4& transform = m_selected_entityID->GetComponent<TransformComponent>();
				ImGui::DragFloat3("Translation", glm::value_ptr(transform[3]), 0.1, -10000, 10000);
				ImGui::TreePop();
			}
		}
		ImGui::End();
	}
	void SceneHierarchyPannel::DrawHierarchyNode(const Entity* ent)
	{
	}
	void SceneHierarchyPannel::DrawProperties()
	{
	}
}