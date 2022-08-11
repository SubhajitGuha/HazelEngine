#pragma once
#include"Hazel/Core.h"
#include"Hazel/Log.h"
#include "imgui.h"
#include "Hazel/Layer.h"

#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/KeyEvent.h"
#include "Hazel/Events/MouseEvent.h"

namespace Hazel {
	class HAZEL_API ImGuiLayer :public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();
		 void OnAttach()override;
		 void OnDetach()override;
		 void OnUpdate() override;
		 void OnEvent(Event& e) override;
	private:
		bool OnMouseScroll(MouseScrollEvent& EventScroll);
		bool OnMouseButtonPressed(MouseButtonPressed& MousePressed);
		bool OnMouseButtonReleased(MouseButtonReleased& MouseReleased);
		bool OnMouseCursorPositionChange(MouseMoveEvent& CursorPos);
		bool OnKeyPressed(KeyPressedEvent& KeyPressed);
		bool OnKeyReleased(KeyReleasedEvent& KeyReleased);
		bool OnKeyTyped(KeyTypedEvent& e);
		bool OnWindowResizeEvent(WindowResizeEvent& ResizeEvent);
	};
}