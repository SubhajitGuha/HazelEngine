#pragma once
#include "Core.h"

namespace Hazel {
	class HAZEL_API Input {
	public:
		static bool IsKeyPressed(int KeyCode) { return m_Input->IsKeyPressedImpl(KeyCode); }
		static bool IsButtonPressed(int Button) { return m_Input->IsMouseButtonPressed(Button); }
		static std::pair<double, double> GetCursorPosition() { return m_Input->GetMousePos(); }
	private:
		virtual bool IsKeyPressedImpl(int keyCode) = 0;
		virtual bool IsMouseButtonPressed(int Button) = 0;
		virtual std::pair<double, double> GetMousePos() = 0;
		static Input* m_Input;
	};
}