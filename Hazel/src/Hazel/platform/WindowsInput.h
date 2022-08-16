#pragma once
#include"Hazel/Core.h"
#include "Hazel/Input.h"
#include "GLFW/glfw3.h"

namespace Hazel {
	class HAZEL_API WindowsInput : public Input
	{
	public:
		 bool IsKeyPressedImpl(int keyCode) override;
		 bool IsMouseButtonPressed(int Button) override;
		 std::pair<double, double> GetMousePos() override;
	};
}
